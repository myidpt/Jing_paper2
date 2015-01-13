/*
 * PrioritySimpleQ.cc
 *
 *  Created on: Apr 23, 2013
 *      Author: yonggang
 */

#include <iostream>
#include <map>
#include "Inputfile.h"
#include "PrioritySimpleQ.h"
#define NOW SIMTIME_DBL(simTime())

using namespace std;

PrioritySimpleQ::PrioritySimpleQ(int numcms, int numsensors) {
    // Init the queue.
    rtTaskQ = new list<ITask *>();
    nrtTaskQ = new list<ITask *>();

    setNumCMs(numcms);
    setNumSensors(numsensors);
    imfCalculator = new IMF(numcms, numsensors);
}

bool PrioritySimpleQ::setNumCMs(int numcms) {
    if (MAX_CM < numcms) {
        cerr << "MAX_CM: " << MAX_CM << " is smaller than numCM: "
             << numcms << endl;
        return false;
    }
    numCMs = numcms;
    return true;
}

bool PrioritySimpleQ::setNumSensors(int numsensors) {
    if (MAX_SENSORS < numsensors) {
        cerr << "MAX_SENSORS: " << MAX_SENSORS
             << " is smaller than numSensros: " << numsensors << endl;
        return false;
    }
    numSensors = numsensors;
    return true;
}

bool PrioritySimpleQ::setCMStatus(IStatus * status[MAX_CM]) {
    for (int i = 0; i < numCMs; i ++) {
        CMStatus[i] = status[i];
    }
    return true;
}

bool PrioritySimpleQ::setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]) {
    imfCalculator->setCMSensors(sensors);
    return true;
}

bool PrioritySimpleQ::setAverageWorkloads(double workloads[MAX_SENSORS]) {
    for (int i = 0; i < numSensors; i ++) {
        averageWorkloads[i] = workloads[i];
    }
    imfCalculator->setWorkloads(averageWorkloads);
    return true;
}

bool PrioritySimpleQ::isEmpty() {
    return rtTaskQ->empty() && nrtTaskQ->empty();
}

bool PrioritySimpleQ::newArrival(ITask * itask) {
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

ITask * PrioritySimpleQ::dispatchNext() {
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

        // For both RT & NRT tasks, iterate with IMF from low to high.
        multimap<double, int> imfmap = imfCalculator->getIMF();
        multimap<double, int>::iterator imfit;

        // For the RT tasks.
        if (isRealtime) {
            if (NOW != task->getArrivalTime()) {
                cerr << NOW << " RT task#" << task->getId()
                     << " is delayed." << endl;
                fflush(stdout);
                fflush(stderr);
            }
            int id = assignNodeForRT(task->getSensorId(), &imfmap);
            if (id == -1) {
                continue;
            }
            return task->createSubTask(1, CMStatus[id]);
            // CMStatus is also updated.
        }

        for (imfit = imfmap.begin(); imfit != imfmap.end(); imfit ++) {
            int nodeid = imfit->second;
            if (CMStatus[nodeid]->isAvailable()
                && CMStatus[nodeid]->hasSensor(sensorid)) {
                // The CM is idle, has the sensor.
                // CMStatus is updated in sreateSubTask.
                return task->createSubTask(1, CMStatus[nodeid]);
            }
        }
    }
    return NULL;
}

int PrioritySimpleQ::assignNodeForRT(int sid, multimap<double, int> * imfmap) {
    // First find the idle nodes
    // (ones that finish at the time but are later in the queue).
    multimap<double, int>::iterator imfit;
    for (imfit = imfmap->begin(); imfit != imfmap->end(); imfit ++) {
        int nodeid = imfit->second;
        if (CMStatus[nodeid]->hasSensor(sid)
            && CMStatus[nodeid]->isAvailable()) {
            return nodeid;
        }
    }

    // Second interrupt the nodes with NRT tasks.
    for (imfit = imfmap->begin(); imfit != imfmap->end(); imfit ++) {
        int nodeid = imfit->second;
        if (CMStatus[nodeid]->hasSensor(sid)
            && CMStatus[nodeid]->getTask() != NULL) {
            SimpleSubTask * curtask =
                (SimpleSubTask *)(CMStatus[nodeid]->getTask());
            if (!curtask->realTime) {
                // Revert the subtask in father task.
                ((SimpleTask *)curtask->getFatherTask())->
                    revertSubTask(curtask);
                return nodeid;
            }
        }
    }

    // If you can't find any node, then an RT task is delayed.
    return -1;
}

bool PrioritySimpleQ::finishedTask(ITask * task) {
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
        return true;
    }
    else {
        return false;
    }
}

PrioritySimpleQ::~PrioritySimpleQ() {
    if (rtTaskQ) {
        delete rtTaskQ;
        rtTaskQ = NULL;
    }
    if (nrtTaskQ) {
        delete nrtTaskQ;
        nrtTaskQ = NULL;
    }
}
