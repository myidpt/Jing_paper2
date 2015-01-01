/*
 * Task.cpp
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include "SimpleTask.h"

int SimpleTask::rtInitId = 10000;

SimpleTask::SimpleTask() {
    subId = 0;
    concurrency = 0;
    subTasks = new list<ITask *>();
}

cObject * SimpleTask::dup() {
    SimpleTask * task = new SimpleTask();
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
    subtask->maxDelay = maxDelay;
    subtask->setServerId(sstatus->getId());
    subtask->realTime = realTime;
    concurrency ++;
    sstatus->assignTask(subtask);

    subTasks->push_front(subtask);

    return subtask;
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
    for (it = subTasks->begin(); it != subTasks->end(); it ++) {
        remainingCost += (*it)->getRemainingCost();
    }
    return remainingCost;
}

double SimpleTask::getRemainingTimeBeforeDeadline() {
    return maxDelay + arrivalTime - SIMTIME_DBL(simTime());
}

double SimpleTask::getDeadline() {
    return maxDelay + arrivalTime;
}

double SimpleTask::getMaxDelay() {
    return maxDelay;
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

    subTasks->remove(task);
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

bool SimpleTask::parseTaskString(string & line) {
    if (sscanf(line.c_str(), "%d %lf %d %d %lf %lf %lf %lf",
        &Id, &arrivalTime, &totalSubTasks, &sensorId,
        &inputData, &outputData, &computeCost, &maxDelay) == 8) {
        undispatchedSubTasks = totalSubTasks;
        unfinishedSubTasks = totalSubTasks;
//        printInformation();
        return true;
    } else {
        return false;
    }
}

void SimpleTask::set(double time, int num, int sid, double cc) {
    Id = rtInitId ++;
    arrivalTime = time;
    totalSubTasks = num;
    sensorId = sid;
    computeCost = cc;
    maxDelay = 0;
    inputData = 1; // temp
    outputData = 1; // temp
    undispatchedSubTasks = totalSubTasks;
    unfinishedSubTasks = totalSubTasks;
//    printInformation();
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

void SimpleTask::printInformation() {
//    cout << "Id = " << Id <<
//        ", arrivalTime = " << arrivalTime << ", totalSubTasks = " << totalSubTasks <<
//        ", sensorId = " << sensorId << ", inputData = " << inputData <<
//        ", outputData = " << outputData << ", computeCost = " << computeCost <<
//        ", maxDelay = " << maxDelay << endl;

    cout << "SimpleTask, Id = " << Id <<
            ", arrivalTime = " << arrivalTime << endl;
}

SimpleTask::~SimpleTask() {
    if (subTasks != NULL) {
        delete subTasks;
        subTasks = NULL;
    }
}

