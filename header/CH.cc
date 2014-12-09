/*
 * Maintain all the status of the CH.
 */

#include "CH.h"

Define_Module(CH);

void CH::initialize()
{
    numCMs = par("numCM").longValue();
    numSensors = par("numSensor").longValue();
    printStatusStep = par("printStatusStep").longValue();

    if (numCMs > MAX_CM) {
        cerr << "Error: cluster member total number exceeds limit: " << MAX_CM << endl;
        endSimulation();
    }

    for (int i = 0; i < MAX_CM; i ++) {
        CMStatus[i] = NULL;
    }

    string taskifilename = par("CH_task_input_filename").stdstringValue();
    string taskofilename = par("CH_task_output_filename").stdstringValue();
    string statusifilename = par("CM_status_input_filename").stdstringValue();
    string statusofilename = par("CM_status_output_filename").stdstringValue();
    string rtstatsfilename = par("CH_rt_task_stats").stdstringValue();
    double period = par("period").doubleValue();
    double chargeRate = par("chargeRate").doubleValue();
    double maxPower = par("maxPower").doubleValue();
    algorithmName = par("algorithm").stdstringValue();

    // Init taskFactory.
    if (!algorithmName.compare("Reserved")) {
        taskFactory = new TaskFactory(
                taskifilename, rtstatsfilename, ITask::SimpleTaskType, period);
    }
    else {
        taskFactory = new TaskFactory(taskifilename, ITask::SimpleTaskType);
    }

    // Init the average workloads record.
    if(! taskFactory->getInitialAverageWorkloads(averageWorkloads)) {
        cout << "Average workloads not initialized." << endl;
    }

    allTraceRead = false;

    // Init statusFactory.
    statusFactory = new StatusFactory(
            statusifilename, StatusFactory::SimpleStatusType,
            period, chargeRate, maxPower);
    distributeInitialStatus();

    //((SimpleStatus *)(CMStatus[0]))->test();

    // Init the queue.
    if (!algorithmName.compare("Balanced")) {
        queue = new BalancedQ(numCMs, numSensors);
        queue->setAverageWorkloads(averageWorkloads);
        queue->setCMSensors(CMSensors);
        queue->setCMStatus(CMStatus);
    }
    else if (!algorithmName.compare("Simple")) {
        queue = new SimpleQ(numCMs, numSensors);
        queue->setAverageWorkloads(averageWorkloads);
        queue->setCMSensors(CMSensors);
        queue->setCMStatus(CMStatus);
    }
    else if (!algorithmName.compare("Reserved")) {
        queue = new ReservedQ(numCMs, numSensors, period, chargeRate);
        queue->setAverageWorkloads(averageWorkloads);
        queue->setCMStatus(CMStatus);
        queue->readTaskStats(rtstatsfilename.c_str());
    }

    // Init writers.
    taskWriter = new CHTaskWriter(taskofilename);
    cmStatusWriter = new CMStatusWriter(statusofilename);

    // Init print timer.
    printStatusTimer = new cPacket("PRINT_STATUS", PRINT_STATUS);
    scheduleAt(printStatusStep, printStatusTimer);

    // Init the self next task timer message. This message is only used for reading new tasks.
    selfNextTaskTimer = new cPacket("TASK_SELF", TASK_SELF);
    // Start to read next task.
    getNextTask();
}

void CH::handleMessage(cMessage *msg)
{
    cPacket * packet = (cPacket *)msg;
    switch(packet->getKind()) {
    case TASK_SELF:
        addWaitingTask();
        processTasks();
        getNextTask();
        break;
    case TASK_RESP:
        processFinishedTasks(packet);
        processTasks();
        break;
    case PRINT_STATUS:
        printStatus();
        break;
    default:
        cerr << "CH: packet kind is unrecognizable: " << packet->getKind() << endl;
    }
}

void CH::distributeInitialStatus() {
    IStatus * status;
    cout << "CH: distributing status to CMs." << endl;
    while ((status = statusFactory->createStatus()) != NULL) {
        cPacket * packet = new cPacket("InitialStatus", STATUS);
        CMStatus[status->getId()] = status; // Record this status locally.
        status->getSensors(CMSensors[status->getId()]);
        cMsgPar * par = new cMsgPar(STATUS_PAR);
        par->setPointerValue(status);
        packet->addPar(par);
        sendSafe(status->getId(), packet);
    }
    for (int i = 0; i < numCMs; i ++) {
        if (CMStatus[i] == NULL) {
            cerr << "Error: CM status " << i << " is not set." << endl;
            endSimulation();
        }
    }
}

// Read the tasks from the file, and put them into the queue.
void CH::getNextTask() {
    ITask * task = taskFactory->createTask();
    if(task == NULL) {
        cout << "CH: All trace read." << endl;
        allTraceRead = true;
        return;
    }

    if (SIMTIME_DBL(simTime()) >= task->getArrivalTime()) {
        queue->newArrival(task);
        processTasks();
        getNextTask();
    }
    else {
        cMsgPar * par = NULL;
        if (! selfNextTaskTimer->hasPar(TASK_PAR)) {
            par = new cMsgPar(TASK_PAR);
            selfNextTaskTimer->addPar(par);
        }
        else {
            par = &(selfNextTaskTimer->par(TASK_PAR));
        }
        par->setPointerValue(task);
        scheduleAt(task->getArrivalTime(), selfNextTaskTimer);
    }
}

void CH::addWaitingTask() {
    cMsgPar par = selfNextTaskTimer->par(TASK_PAR);
    ITask * task = (ITask *)(par.pointerValue());
    queue->newArrival(task);
}

// Process the tasks. Dispatch the queued sub-tasks to the idle nodes.
void CH::processTasks() {
    while (true) {
        ITask * subtask = queue->dispatchNext();
        if (subtask == NULL) {
            break;
        }
        cPacket * packet = new cPacket("Task", TASK_REQ);
        cMsgPar * par = new cMsgPar(TASK_PAR);
        par->setPointerValue(subtask);
        packet->addPar(par);
        packet->setBitLength(subtask->getInputData() * MB_TO_BIT);
        sendSafe(subtask->getServerId(), packet);
        cout << "CH: assign task id = " << subtask->getId()
             << " to CM#" << subtask->getServerId() << endl;
    }
}

void CH::processFinishedTasks(cPacket * packet) {
    cMsgPar par = packet->par(TASK_PAR);
    ITask * task = (ITask *)(par.pointerValue());
    ITask * fathertask = task->getFatherTask();

    if (fathertask == NULL) {
        cerr << "Fathertask == NULL." << endl;
        return;
    }

    cout << "CH: finished task id = " << task->getId()
         << " from CM#" << task->getServerId() << endl;

    if (queue->finishedTask(task)) {
        taskWriter->writeTask(fathertask);
        delete fathertask;
    }
    delete task;
    delete packet;

    if (allTraceRead && queue->isEmpty()) {
        endSimulation();
    }
}

void CH::printStatus() {
    //cmStatusWriter->writeStatus(CMStatus, numCMs);
    scheduleAt(SIMTIME_DBL(simTime()) + printStatusStep, printStatusTimer);
}

void CH::sendSafe(int id, cPacket * packet) {
    cChannel * cch = gate("gate$o", id)->getTransmissionChannel();
    if(cch->isBusy()) {
        sendDelayed(packet, cch->getTransmissionFinishTime() - simTime(), "gate$o", id);
    }
    else {
        send(packet, "gate$o", id);
    }
}

void CH::finish() {
    for (int i = 0; i < numCMs; i ++) {
        if (CMStatus[i]) {
            delete CMStatus[i];
            CMStatus[i] = NULL;
        }
    }
    if (selfNextTaskTimer) {
        cancelAndDelete(selfNextTaskTimer);
        selfNextTaskTimer = NULL;
    }
    if (printStatusTimer) {
        cancelAndDelete(printStatusTimer);
        printStatusTimer = NULL;
    }
    if (taskWriter) {
        delete taskWriter;
        taskWriter = NULL;
    }
    if (cmStatusWriter) {
        delete cmStatusWriter;
        cmStatusWriter = NULL;
    }
}