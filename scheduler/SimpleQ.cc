/*
 * SimpleQ.cc
 *
 *  Created on: Apr 22, 2013
 *      Author: yonggang
 */

#include "SimpleQ.h"

SimpleQ::SimpleQ(int numcms, int numsensors) {
    // Init the queue.
    taskQ = new list<SimpleTask *>();

    setNumCMs(numcms);
    setNumSensors(numsensors);
    // Init the idle time indicator for each CM.
    for (int i = 0; i < MAX_CM; i ++) {
        CMIdleTime[i] = IDLE_SIG; // Idle now.
    }
    srand(time(NULL));
}

bool SimpleQ::setNumCMs(int numcms) {
    if (MAX_CM < numCMs) {
        cerr << "MAX_CM: " << MAX_CM << " is smaller than numCM: " << numCMs << endl;
        return false;
    }
    numCMs = numcms;
    return true;
}

bool SimpleQ::setNumSensors(int numsensors) {
    if (MAX_SENSORS < numsensors) {
        cerr << "MAX_SENSORS: " << MAX_SENSORS << " is smaller than numSensros: " << numsensors << endl;
        return false;
    }
    numSensors = numsensors;
    return true;
}

bool SimpleQ::setCMStatus(IStatus * status[MAX_CM]) {
    for (int i = 0; i < MAX_CM; i ++) {
        CMStatus[i] = status[i];
    }
    return true;
}

bool SimpleQ::setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]) {
    for (int i = 0; i < MAX_CM; i ++) {
        for (int j =0; j < MAX_SENSORS; j ++) {
            CMSensors[i][j] = sensors[i][j];
        }
    }
    return true;
}

bool SimpleQ::setAverageWorkloads(double workloads[MAX_SENSORS]) {
    for (int i = 0; i < MAX_SENSORS; i ++) {
        averageWorkloads[i] = workloads[i];
    }
    return true;
}

bool SimpleQ::isEmpty() {
    return taskQ->empty();
}

bool SimpleQ::newArrival(ITask * task) {
    taskQ->push_back((SimpleTask *)task);
    task->setParameter(SimpleTask::PARADEGREE, DEGREE);
    return true;
}

ITask * SimpleQ::dispatchNext() {
    // Test if there are idle nodes first.
    bool hasidle = false;
    for (int i = 0; i < numCMs; i ++) { // For the CMs.
        if (CMIdleTime[i] < 0 && CMIdleTime[i] > IDLE_DEAD_BOUND) {
            hasidle = true;
            break;
        }
    }
    if (! hasidle) {
        return NULL;
    }

    list<SimpleTask *>::iterator it;
    for (it = taskQ->begin(); it != taskQ->end(); it ++) { // For tasks in queue.
        if ((*it)->dispatched()) {
            continue;
        }
        int sensorid = (*it)->getSensorId(); // Get the required sensor ID of the task.

        double randf = ((double)rand()) / RAND_MAX;
        int start = randf * numCMs;
        bool round = false;

        for (int i = start; ; i ++) { // For the CMs.
            if (i == start) { // Judge the break condition. A round is enough.
                if (round) {
                    break;
                }
                else {
                    round = true;
                }
            }
            if (i == numCMs) { // Loop back.
                i = -1;
                continue;
            }
            if (CMIdleTime[i] < 0 && CMIdleTime[i] > IDLE_DEAD_BOUND
                    && CMSensors[i][sensorid]) {
                // The CM is idle and the CM has the sensor.
                // Assign the subtask to the CM.
                ITask * subtask = (*it)->createSubTask(1, CMStatus[i]);
                if (subtask == NULL) {
                    cerr << "Not right here." << endl;
                }
                subtask->setServerId(i);
                CMIdleTime[i] = SIMTIME_DBL(simTime()) +
                        subtask->getComputeCost() / CMStatus[i]->getComputeCap();
                return subtask;
            }
        }
    }
    return NULL;
}

bool SimpleQ::finishedTask(ITask * task) {
    SimpleTask * fathertask = (SimpleTask *)(task->getFatherTask());
    CMIdleTime[task->getServerId()] = IDLE_SIG; // Set CM idle.
    int cmid = task->getServerId();
    if (CMStatus[cmid]->getPower() <= 0) { // This node is exhausted.
        setCMDead(cmid);
    }
    if (fathertask->setFinishedSubTask(task)) { // Remove the father task from queue.
        taskQ->remove(fathertask);
        return true;
    }
    else {
        return false;
    }
}

void SimpleQ::setCMDead(int cmid) {
    CMIdleTime[cmid] = DEAD_SIG;
    for (int i = 0; i < numSensors; i ++) {
        CMSensors[cmid][i] = false;
    }
}


SimpleQ::~SimpleQ() {
    if (taskQ) {
        delete taskQ;
        taskQ = NULL;
    }
}

