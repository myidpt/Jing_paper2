
#include "SimpleSubTask.h"
#include "CM.h"

Define_Module(CM);

int CM::idInit = 0;

void CM::initialize()
{
    status = NULL;
    taskAtService = NULL;
    myId = idInit;
    idInit ++;
}

void CM::handleMessage(cMessage *msg)
{
    cPacket * packet = (cPacket *)msg;
    switch (packet->getKind()) {
    case STATUS:
        processStatus(packet);
        break;
    case TASK_REQ:
        processTask(packet);
        break;
    case TASK_COMP:
        processFinishedTask(packet);
        break;
    default:
        cerr << "CM #" << myId << ": unknown packet kind: "
             << packet->getKind() << endl;
    }
}

void CM::processTask(cPacket * packet) {
    if (taskAtService != NULL) {
        cMsgPar par = packet->par(TASK_PAR);
        ITask * task = (ITask *)(par.pointerValue());
        cerr << "CM #" << myId << ": busy when received new task request id = "
             << task->getId() << endl;
        endSimulation();
        return;
    }
    cMsgPar par = packet->par(TASK_PAR);
    ITask * task = (ITask *)(par.pointerValue());
    taskAtService = packet;
    packet->setKind(TASK_COMP);
    if (task->getTaskType() == ITask::SimpleSubTaskType) {
        task->setServiceTime(SIMTIME_DBL(simTime()));
        double timespan = task->getComputeCost() / status->getComputeCap();
        scheduleAt(SIMTIME_DBL(simTime()) + timespan, packet);
    }
    else {
        cerr << "CM::processTask: unsupported task type." << endl;
    }
}

void CM::processFinishedTask(cPacket * packet) {
    taskAtService = NULL;
    packet->setKind(TASK_RESP);
    cMsgPar par = packet->par(TASK_PAR);
    ITask * task = (ITask *)(par.pointerValue());
    packet->setBitLength(task->getOutputData() * MB_TO_BIT);
    sendSafe(packet);
}

void CM::processStatus(cPacket * packet) {
    cMsgPar par = packet->par(STATUS_PAR);
    status = (IStatus *)(par.pointerValue());
    delete packet;
}


void CM::sendSafe(cPacket * packet){
    cChannel * cch = gate("gate$o")->getTransmissionChannel();
    if(cch->isBusy()) {
        sendDelayed(packet, cch->getTransmissionFinishTime() - simTime(), "gate$o");
    }
    else {
        send(packet, "gate$o");
    }
}
