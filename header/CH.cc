/*
 * Maintain all the status of the CH.
 */

#include <string>
#include <omnetpp.h>
#include <list>
#include "task/SimpleTask.h"
#include "iostreamer/ostreamer/TaskWriter.h"
#include "scheduler/IQueue.h"
#include "scheduler/SimpleQ.h"
#include "scheduler/PrioritySimpleQ.h"
#include "scheduler/BalancedQ.h"
#include "scheduler/ReservedQ.h"
#include "CH.h"

Define_Module(CH);

#define NOW SIMTIME_DBL(simTime())
#define DEBUG

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

    string nrttaskifilename = par("CH_task_input_filename").stdstringValue();
    string taskofilename = par("CH_task_output_filename").stdstringValue();
    string statusifilename = par("CM_status_input_filename").stdstringValue();
    string statusofilename = par("CM_status_output_filename").stdstringValue();
    string rttaskifilename = par("CH_rt_task_stats").stdstringValue();
    period = par("period").doubleValue();
    double chargeRate = par("chargeRate").doubleValue();
    double maxPower = par("maxPower").doubleValue();
    algorithmName = par("algorithm").stdstringValue();

    // Init taskFactory.
    if (!algorithmName.compare("Balanced") || !algorithmName.compare("Simple")) {
        taskFactory = new TaskFactory(nrttaskifilename, ITask::SimpleTaskType);
    }
    else {
        taskFactory = new TaskFactory(
                nrttaskifilename, rttaskifilename,
                ITask::SimpleTaskType, period);
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
    else if (!algorithmName.compare("PrioritySimple")) {
        queue = new PrioritySimpleQ(numCMs, numSensors);
        queue->setAverageWorkloads(averageWorkloads);
        queue->setCMSensors(CMSensors);
        queue->setCMStatus(CMStatus);
    }
    else if (!algorithmName.compare("Reserved")) {
        queue = new ReservedQ(numCMs, numSensors, period, chargeRate);
        queue->setAverageWorkloads(averageWorkloads);
        queue->setCMSensors(CMSensors);
        queue->setCMStatus(CMStatus);
        queue->readTaskStats(rttaskifilename.c_str());
    }

    // Init writers.
    taskWriter = new TaskWriter(taskofilename);

    // Init print timer.
    printStatusTimer = new cPacket("PRINT_STATUS", PRINT_STATUS);
    scheduleAt(printStatusStep, printStatusTimer);

    // Init the self next task timer message. This message is only used for reading new tasks.
    selfNextTaskTimer = new cPacket("TASK_SELF", TASK_SELF);
    // Start to read next task.
    getNextTask();

    tickCheckerTick = 10; // Sec.
    selfTickChecker = new cPacket("TICK_CHECKER", TICK_CHECKER);
    processTickChecker();
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
    case TICK_CHECKER:
        processTickChecker();
        break;
    default:
        cerr << "CH: packet kind is unrecognizable: " << packet->getKind() << endl;
    }
}

void CH::distributeInitialStatus() {
    IStatus * status;
    cout << NOW << " CH: distributing status to CMs." << endl;
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

// Read the RT or NRT tasks from taskFactory, and put them into the queue.
void CH::getNextTask() {
    ITask * task = taskFactory->createTask();
    if(task == NULL) {
        cout << "CH: All traces read." << endl;
        allTraceRead = true;
        return;
    }

    if (NOW >= task->getArrivalTime()) {
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
    ITask * task = (ITask *)(selfNextTaskTimer->par(TASK_PAR).pointerValue());
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
#ifdef DEBUG
        cout << NOW << " CH: assign task id = " << subtask->getId()
             << " to CM#" << subtask->getServerId()
             << " (RT=" << subtask->realTime << ")"
             << " power=" << CMStatus[subtask->getSensorId()]->getPower() << endl;
#endif
    }
}

void CH::processFinishedTasks(cPacket * packet) {
    cMsgPar par = packet->par(TASK_PAR);
    ITask * task = (ITask *)(par.pointerValue());
    SimpleTask * fathertask = (SimpleTask *)(task->getFatherTask());

    if (fathertask == NULL) {
        cerr << "Fathertask == NULL." << endl;
        return;
    }
#ifdef DEBUG
    cout << NOW << " CH: finished task id = " << task->getId()
         << " from CM#" << task->getServerId() << endl;
#endif
    if (queue->finishedTask(task)) {
        taskWriter->writeSimpleTask(fathertask);
        delete fathertask;
    }
    delete task;
    delete packet;

    if (allTraceRead && queue->isEmpty()) {
        cout << "allTraceRead && queue is Empty" << endl;
        endSimulation();
    }
}

void CH::processTickChecker() {
    double offset = NOW - ((int)(NOW / period) * period);
    if (offset < period/2) {
        scheduleAt(NOW + tickCheckerTick, selfTickChecker);
    }
    else {
        scheduleAt((int)(NOW / period + 1) * period, selfTickChecker);
    }
    processTasks();
}

void CH::printStatus() {
    scheduleAt(NOW + printStatusStep, printStatusTimer);
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
    if (selfTickChecker) {
        cancelAndDelete(selfTickChecker);
        selfTickChecker = NULL;
    }
    if (taskWriter) {
        delete taskWriter;
        taskWriter = NULL;
    }
}
