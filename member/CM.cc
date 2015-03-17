#include <string>
#include "SimpleSubTask.h"
#include "CM.h"
#include "iostreamer/ostreamer/StatusWriter.h"
#include "iostreamer/ostreamer/TaskWriter.h"
#include "task/SimpleSubTask.h"

#define NOW SIMTIME_DBL(simTime())

Define_Module(CM);

int CM::idInit = 0;

void CM::initialize()
{
    status = NULL;
    taskAtService = NULL;
    myId = idInit;
    idInit ++;

    string task_file_prefix = par(
        "task_output_filename").stdstringValue();
    string status_file_prefix = par(
        "status_output_filename").stdstringValue();
    period = par("period").doubleValue();
    string task_file = task_file_prefix + "0000";
    string status_file = status_file_prefix + "0000";
    int tf_sb = task_file_prefix.length(); // The start bit of the digit area.
    int sf_sb = status_file_prefix.length(); // The start bit of the digit area.
    task_file[tf_sb] = myId / 1000 + '0';
    status_file[sf_sb] = myId / 1000 + '0';
    task_file[tf_sb + 1] = myId % 1000 / 100 + '0';
    status_file[sf_sb + 1] = myId % 1000 / 100 + '0';
    task_file[tf_sb + 2] = myId % 100 / 10 + '0';
    status_file[sf_sb + 2] = myId % 100 / 10 + '0';
    task_file[tf_sb + 3] = myId % 10 + '0';
    status_file[sf_sb + 3] = myId % 10 + '0';

    statusWriter = new StatusWriter(status_file);
    taskWriter = new TaskWriter(task_file);

    cPacket * pPacket = new cPacket("DAY-NIGHT");
    pPacket->setKind(DAY_NIGHT);
    scheduleAt(period/2, pPacket);
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
    case DAY_NIGHT:
        processDayNightMsg(packet);
        break;
    default:
        cerr << "CM #" << myId << ": unknown packet kind: "
             << packet->getKind() << endl;
    }
}

void CM::processTask(cPacket * packet) {
    cMsgPar par = packet->par(TASK_PAR);
    ITask * task = (ITask *)(par.pointerValue());
    if (taskAtService != NULL) {
        if (!task->realTime) {
            cerr << "CM #" << myId << ": busy when received new NRT task id = "
                 << task->getId() << ". Error!" << endl;
            return;
        }
        cancelEvent(taskAtService);
        SimpleSubTask * oldtask =
            (SimpleSubTask *)(taskAtService->par(TASK_PAR).pointerValue());
        taskWriter->writeUnfinishedSimpleSubTask(oldtask);
        delete oldtask;
    }
    taskAtService = packet;
    packet->setKind(TASK_COMP);
    if (task->getTaskType() == ITask::SimpleSubTaskType) {
        task->setServiceTime(NOW);
        double finishtime = NOW + task->getComputeCost() / status->getComputeCap();
        scheduleAt(finishtime, packet);
        task->setFinishTime(finishtime);
    }
    else {
        cerr << "CM::processTask: unsupported task type." << endl;
    }
    statusWriter->writeStatus(status);
}

void CM::processFinishedTask(cPacket * packet) {
    taskAtService = NULL;
    packet->setKind(TASK_RESP);
    cMsgPar par = packet->par(TASK_PAR);
    ITask * task = (ITask *)(par.pointerValue());
    packet->setBitLength(task->getOutputData() * MB_TO_BIT);
    sendSafe(packet);
    statusWriter->writeStatus(status);
    taskWriter->writeSimpleSubTask((SimpleSubTask *)task);
}

void CM::processStatus(cPacket * packet) {
    cMsgPar par = packet->par(STATUS_PAR);
    status = (IStatus *)(par.pointerValue());
    delete packet;
    statusWriter->writeStatus(status);
}

void CM::processDayNightMsg(cPacket * packet) {
    statusWriter->writeStatus(status);
    scheduleAt(NOW + period/2, packet);
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

void CM::finish() {
    delete taskWriter;
    delete statusWriter;
}
