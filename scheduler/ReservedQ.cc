/*
 * ReservedQ.cc
 *
 *  Created on: Apr 23, 2013
 *      Author: yonggang
 */

#include <iostream>
#include <map>
#include "Inputfile.h"
#include "ReservedQ.h"
#define NOW SIMTIME_DBL(simTime())

using namespace std;

ReservedQ::ReservedQ(int numcms, int numsensors, double p, double cr) {
    // Init the queue.
    rtTaskQ = new list<ITask *>();
    nrtTaskQ = new list<ITask *>();

    setNumCMs(numcms);
    setNumSensors(numsensors);
    period = p;
    chargeRate = cr;
    rtReserv = new RTReservation(numcms, numsensors, period, chargeRate);
    imfCalculator = new IMF(numcms, numsensors);
}

bool ReservedQ::setNumCMs(int numcms) {
    if (MAX_CM < numcms) {
        cerr << "MAX_CM: " << MAX_CM << " is smaller than numCM: "
             << numcms << endl;
        return false;
    }
    numCMs = numcms;
    return true;
}

bool ReservedQ::setNumSensors(int numsensors) {
    if (MAX_SENSORS < numsensors) {
        cerr << "MAX_SENSORS: " << MAX_SENSORS
             << " is smaller than numSensros: " << numsensors << endl;
        return false;
    }
    numSensors = numsensors;
    return true;
}

bool ReservedQ::setCMStatus(IStatus * status[MAX_CM]) {
    for (int i = 0; i < numCMs; i ++) {
        CMStatus[i] = status[i];
    }
    rtReserv->setStatus(status);
    return true;
}

bool ReservedQ::setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]) {
    imfCalculator->setCMSensors(sensors);
    return true;
}

bool ReservedQ::setAverageWorkloads(double workloads[MAX_SENSORS]) {
    for (int i = 0; i < numSensors; i ++) {
        averageWorkloads[i] = workloads[i];
    }
    imfCalculator->setWorkloads(averageWorkloads);
    return true;
}

void ReservedQ::readTaskStats(const char  * filename) {
    rtReserv->makeReservation(filename);
}

bool ReservedQ::isEmpty() {
    return rtTaskQ->empty() && nrtTaskQ->empty();
}

bool ReservedQ::newArrival(ITask * itask) {
    SimpleTask * task = (SimpleTask *)itask;
    task->setParameter(SimpleTask::PARADEGREE, task->getTotalSubTasks());
    if (task->realTime) { // No deadline for real time tasks.
        rtTaskQ->push_back(task);
    }
    else { // Deadlines exist for non-real-time tasks.
        double deadline = task->getDeadline();
        list<ITask *>::iterator it;
        for (it = nrtTaskQ->begin(); it != nrtTaskQ->end(); it ++) {
            if ((*it)->getDeadline() > deadline) {
                nrtTaskQ->insert(it, task);
            }
        }
        if (it == nrtTaskQ->end()) {
            nrtTaskQ->push_back(task);
        }
    }
    return true;
}

ITask * ReservedQ::dispatchNext() {
    // Test if there are idle CMs.
    bool hasidle = false;
    for (int i = 0; i < numCMs; i ++) { // For the CMs.
        if (CMStatus[i]->isAvailable()) {
            hasidle = true;
            break;
        }
    }
    if (!hasidle) {
        return NULL;
    }

    // Set the new power of each CM for the IMF calculator.
    double power[MAX_CM];
    for (int i = 0; i < numCMs; i ++) {
        power[i] = CMStatus[i]->getPower();
    }
    imfCalculator->setCMPower(power);

    list<ITask *>::iterator it;
    // In the following loop, we iterate rtTaskQ first, then nrtTaskQ.
    bool isRealtime = true;
    for (it = rtTaskQ->begin(); it != nrtTaskQ->end(); it ++) {
        if (it == rtTaskQ->end()) {
            it = nrtTaskQ->begin();
            isRealtime = false;
            if (it == nrtTaskQ->end()) {
                break;
            }
        }
        SimpleTask * task = (SimpleTask *)(*it);
        if (task->dispatched()) {
            continue;
        }
        int sensorid = task->getSensorId();
        // Get the required sensor ID of the task.

        if (task->getRemainingCost() == 0) {
            continue; // No need to dispatch.
        }

        // For the RT tasks.
        if (isRealtime) {
            if (NOW != task->getArrivalTime()) {
                cerr << NOW << " RT task#" << task->getId()
                     << " is delayed!" << endl;
                fflush(stdout);
                fflush(stderr);
                while(true);
                return NULL;
            }
            int id = rtReserv->assignNodeForRT(NOW,task->getSensorId());
            if (!CMStatus[id]->hasSensor(sensorid)) {
                // This should never happen.
                cerr << NOW << " Allocating RT Task to CM#" << id
                     << " does not have the sensor!" << endl;
                fflush(stdout);
                fflush(stderr);
                return NULL;
            }
            if (!CMStatus[id]->isAvailable()) {
                // Should be available at the same time,
                // but in later position in the queue.
                // Slot can be secured by finding violations.
                continue;
            }
            rtReserv->confirmAssignment(task->getSensorId());
            return task->createSubTask(1, CMStatus[id]);
            // CMStatus is also updated.
        }

        // For the NRT tasks, iterate with IMF from low to high.
        multimap<double, int> imfmap = imfCalculator->getIMF();
        multimap<double, int>::iterator imfit;
        for (imfit = imfmap.begin(); imfit != imfmap.end(); imfit ++) {
            int nodeid = imfit->second;
            if (CMStatus[nodeid]->isAvailable()
                && CMStatus[nodeid]->hasSensor(sensorid)
                && !rtReserv->findViolationForNode(
                    nodeid, CMStatus[nodeid]->getPower(),
                    NOW, task->getSubTaskCost())) {
                // The CM is idle, has the sensor.
                // CMStatus is updated in sreateSubTask.
                return task->createSubTask(1, CMStatus[nodeid]);
            }
        }
    }
    return NULL;
}

// Write out and delete father task here.
bool ReservedQ::finishedTask(ITask * task) {
    SimpleTask * fathertask = (SimpleTask *)(task->getFatherTask());
    int cmid = task->getServerId();
    CMStatus[cmid]->taskFinish(); // Set CM idle.
    if (fathertask->setFinishedSubTask(task)) {
        // Remove the father task from queue.
        if (task->realTime) {
            rtTaskQ->remove(fathertask);
        }
        else {
            nrtTaskQ->remove(fathertask);
        }
        fathertask->writeOut();
        delete fathertask;
        return true;
    }
    else {
        return false;
    }
}

ReservedQ::~ReservedQ() {
    if (rtTaskQ) {
        delete rtTaskQ;
        rtTaskQ = NULL;
    }
    if (nrtTaskQ) {
        delete nrtTaskQ;
        nrtTaskQ = NULL;
    }
}
