/*
 * Task.cpp
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include "iostreamer/ostreamer/Outputfile.h"
#include "SimpleTask.h"
#define NOW SIMTIME_DBL(simTime())
#define BUFF_SIZE 5000

int SimpleTask::rtInitId = 10000;

SimpleTask::SimpleTask(Outputfile * file)
: canceledSubTasks(0), subId(0), concurrency(0),
  outstandingSubTasks(new list<ITask *>()), outputfile(file) {
}

cObject * SimpleTask::dup() {
    SimpleTask * task = new SimpleTask(outputfile);
    return task;
}

int SimpleTask::getId() {
    return Id;
}

int SimpleTask::getSensorId() {
    return sensorId;
}

double SimpleTask::getArrivalTime() {
    return arrivalTime;
}

double SimpleTask::getFinishTime() {
    return finishTime;
}

void SimpleTask::setFinishTime(double) {
    cerr << "SimpleTask does not have setFinishTime." << endl;
}

Outputfile * SimpleTask::getTaskWriter() {
    return outputfile;
}

// Return NULL if all dispatched.
ITask * SimpleTask::createSubTask(int chunks, IStatus * sstatus) {
    if (undispatchedSubTasks == 0) { // All dispatched.
        return NULL;
    }
    if (chunks > undispatchedSubTasks) {
        chunks = undispatchedSubTasks;
    }
    undispatchedSubTasks -= chunks;
    SimpleSubTask * subtask = new SimpleSubTask(this, chunks, sstatus);
    subtask->Id = this->Id * ID_INDEX + subId;
    subId ++;
    subtask->computeCost = computeCost * chunks / totalSubTasks;
    subtask->inputData = inputData * chunks / totalSubTasks;
    subtask->outputData = outputData * chunks / totalSubTasks;
    subtask->sensorId = sensorId;
    subtask->maxLatency = maxLatency;
    subtask->setServerId(sstatus->getId());
    subtask->realTime = realTime;
    subtask->setFinishTime(NOW + computeCost);
    concurrency ++;
    sstatus->assignTask(subtask);

    outstandingSubTasks->push_front(subtask);

    return subtask;
}

void SimpleTask::revertSubTask(SimpleSubTask * task) {
    if (task->getFatherTask() != this) {
        cerr << "Task's father task is not this." << endl;
    }
    concurrency --;
    outstandingSubTasks->remove(task);
    undispatchedSubTasks ++;
}

ITask * SimpleTask::getFatherTask() {
    cerr << "SimpleTask does not have getFatherTask." << endl;
    return NULL;
}

ITask::TaskType SimpleTask::getTaskType() {
    return ITask::SimpleTaskType;
}

double SimpleTask::getInputData() {
    return inputData;
}

double SimpleTask::getOutputData() {
    return outputData;
}

double SimpleTask::getComputeCost() {
    return computeCost;
}

double SimpleTask::getRemainingCost() {
    return computeCost * undispatchedSubTasks / totalSubTasks
        + getServingWorkload();
}

double SimpleTask::getServingWorkload() {
    double remainingCost = 0;
    list<ITask *>::iterator it;
    for (it = outstandingSubTasks->begin();
         it != outstandingSubTasks->end(); it ++) {
        remainingCost += (*it)->getRemainingCost();
    }
    return remainingCost;
}

double SimpleTask::getRemainingTimeBeforeDeadline() {
    return maxLatency + arrivalTime - SIMTIME_DBL(simTime());
}

double SimpleTask::getDeadline() {
    return maxLatency + arrivalTime;
}

double SimpleTask::getMaxLatency() {
    return maxLatency;
}

int SimpleTask::getServerId() {
    cerr << "SimpleTask does not implement getServerId." << endl;
    return 0;
}

void SimpleTask::setServerId(int id) {
    cerr << "SimpleTask does not implement setServerId." << endl;
}

bool SimpleTask::setParameter(int param, double value) {
    if (param == PARADEGREE) {
        paradegree = value;
        return true;
    }
    return false;
}

int SimpleTask::getTotalSubTasks() {
    return totalSubTasks;
}

double SimpleTask::getParameter(int param) {
    if (param == CONCURRENCY) {
        return concurrency;
    }
    else if (param == PARADEGREE) {
        return paradegree;
    }
    else if (param == DEGREEFULL) {
        if (concurrency >= paradegree) {
            return 1;
        }
        else {
            return -1;
        }
    }
    else if (param == SUBTASK_COST) {
        return computeCost / totalSubTasks;
    }
    return -1;
}

// Return true if all done. Return false if error happens or not all done.
bool SimpleTask::setFinishedSubTask(ITask * task) {
    if (task->getFatherTask() != this) {
        cerr << "Task's father task is not this." << endl;
        return false;
    }
    concurrency --;
    unfinishedSubTasks -= ((SimpleSubTask * )task)->chunks;

    outstandingSubTasks->remove(task);
    subTaskStats.push_back(
        pair<int, double>(task->getServerId(), task->getServiceTime()));
    if (unfinishedSubTasks < 0) {
        cerr << "[" << SIMTIME_DBL(simTime()) << "] ID:" << Id
             << " SimpleTask: unfinishedSubTasks < 0!!" << endl;
        fflush(stdout);
        while(true);
        return false;
    }
    else if (unfinishedSubTasks == 0) {
        finishTime = SIMTIME_DBL(simTime());
        return true;
    }
    return false;
}

// All of the subtasks are dispatched.
bool SimpleTask::dispatched() {
    if (undispatchedSubTasks == 0) {
        return true;
    }
    else {
        return false;
    }
}

// All of the subtasks are finished.
bool SimpleTask::finished() {
    if (unfinishedSubTasks == 0) {
        return true;
    }
    else {
        return false;
    }
}

// For NRT tasks.
bool SimpleTask::parseTaskString(string & line) {
    if (sscanf(line.c_str(), "%d %lf %d %d %lf %lf %lf %lf",
        &Id, &arrivalTime, &totalSubTasks, &sensorId,
        &inputData, &outputData, &computeCost, &maxLatency) == 8) {
        undispatchedSubTasks = totalSubTasks;
        unfinishedSubTasks = totalSubTasks;
        finishTime = arrivalTime;
        canceledSubTasks = 0;
//        printInformation();
        return true;
    } else {
        return false;
    }
}

// For RT task.
void SimpleTask::set(double time, int num, int sid, double cc) {
    Id = rtInitId ++;
    arrivalTime = time;
    finishTime = arrivalTime;
    totalSubTasks = num;
    sensorId = sid;
    computeCost = cc;
    maxLatency = computeCost/num;
    inputData = 1; // temp
    outputData = 1; // temp
    undispatchedSubTasks = totalSubTasks;
    unfinishedSubTasks = totalSubTasks;
    canceledSubTasks = 0;
}

int SimpleTask::getUnfinishedSubTasks() {
    return unfinishedSubTasks;
}

int SimpleTask::getUndispatchedSubTasks() {
    return undispatchedSubTasks;
}

int SimpleTask::getSubTaskCost() {
    return computeCost / totalSubTasks;
}

int SimpleTask::getConcurrency() {
    return concurrency;
}

vector<pair<int, double> > SimpleTask::getSubTaskStats() {
    return subTaskStats;
}

// Return true if all finished.
bool SimpleTask::cancelDelayedSubTasks() {
    // undispatchedSubTasks -> finishedSubTasks.
    canceledSubTasks = undispatchedSubTasks;
    undispatchedSubTasks = 0;
    unfinishedSubTasks -= canceledSubTasks;
    if (unfinishedSubTasks == 0) {
        return true;
    }
    return false;
}

int SimpleTask::getCanceledTasks() {
    return canceledSubTasks;
}

void SimpleTask::printInformation() {
    cout << "SimpleTask, Id = " << Id <<
            ", arrivalTime = " << arrivalTime << endl;
}

void SimpleTask::writeOut() {
    char buff[BUFF_SIZE];
    int pos = sprintf(buff, "%d %.2lf %.2lf %.2lf %d %d",
            Id, arrivalTime, finishTime, finishTime - arrivalTime,
            sensorId, canceledSubTasks);
    if (realTime) {
        pos += sprintf(buff + pos, " R");
    }
    else {
        pos += sprintf(buff + pos, " N");
    }
    double delay =
        finishTime - arrivalTime - maxLatency;
    if (delay < 0) {
        delay = 0;
    }
    sprintf(buff + pos, " %.3lf", delay);
    outputfile->writeLine(string(buff));
    vector<pair<int, double> >::iterator it;
    int position = sprintf(buff, "SUB ");
    for (it = subTaskStats.begin(); it != subTaskStats.end(); it ++) {
        position +=
            sprintf(buff + position, "[%d]%.2lf,", it->first, it->second);
        if (position > BUFF_SIZE) {
            cerr << "Output buff size not enough! "
                 << position << " > " << BUFF_SIZE << endl;
        }
    }
    outputfile->writeLine(string(buff));
}

SimpleTask::~SimpleTask() {
    if (outstandingSubTasks != NULL) {
        delete outstandingSubTasks;
        outstandingSubTasks = NULL;
    }
}

