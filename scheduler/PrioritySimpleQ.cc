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
#include "scheduler/ordering/Ordering.h"
#define NOW SIMTIME_DBL(simTime())
//#define DEBUG

using namespace std;

PrioritySimpleQ::PrioritySimpleQ(int numcms, int numsensors, Ordering * order) {
    // Init the queue.
    rtTaskQ = new list<ITask *>();
    nrtTaskQ = new list<ITask *>();

    setNumCMs(numcms);
    setNumSensors(numsensors);
    imfCalculator = order;
}

bool PrioritySimpleQ::setNumCMs(int numcms) {
    if (MAX_CM < numcms) {
        cerr << "PrioritySimpleQ: MAX_CM: " << MAX_CM << " is smaller than numCM: "
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
    // Set the new power of each CM for the imfcalculator.
    double power[MAX_CM];
    for (int i = 0; i < numCMs; i ++) {
        power[i] = CMStatus[i]->getPower();
    }
    imfCalculator->setCMPower(power);

    list<ITask *>::iterator it;
    // In the following loop, we iterate rtTaskQ first, then nrtTaskQ.
    bool isRealtime = true;
    it = rtTaskQ->begin();
    while (it != nrtTaskQ->end()) {
        if (it == rtTaskQ->end()) {
            it = nrtTaskQ->begin();
            isRealtime = false;
            if (it == nrtTaskQ->end()) {
                break;
            }
        }
        SimpleTask * task = (SimpleTask *)(*it);
        if (task->dispatched()) {
            it ++;
            continue;
        }
        int sensorid = task->getSensorId();
        double cost = task->getSubTaskCost();
        // Get the required sensor ID of the task.

        if (task->getRemainingCost() == 0) {
            it ++;
            continue; // No need to dispatch.
        }

        // For both RT & NRT tasks, iterate with IMF from low to high.
        vector<int> imf = imfCalculator->getOrderingList();
        vector<int>::iterator imfit;

        // For the RT tasks.
        if (isRealtime) {
            if (NOW != task->getArrivalTime()) {
                cerr << "Found undispatched RT task still in RT queue"
                     << " after arrival time!!" << endl;
                it ++;
                continue; // This task is already processed. why coming here?
            }
            int id = assignNodeForRT(sensorid, cost, &imf);
#ifdef DEBUG
            cout << "assigned task " << task->getId() << "'s subtask to " << id << endl;
#endif
            if (id == -1) { // Can't find a node for RT task.
                if (task->cancelDelayedSubTasks()) { // No subtask can be dispatched :(
                    it = rtTaskQ->erase(it); // Returns next element.
                    task->writeOut();
                }
                continue;
            }
            return task->createSubTask(1, CMStatus[id]);
            // CMStatus is also updated.
        }

        // For the NRT tasks.
        for (imfit = imf.begin(); imfit != imf.end(); imfit ++) {
            int nodeid = *imfit;
            if (CMStatus[nodeid]->isAvailable()
                && CMStatus[nodeid]->hasSensor(sensorid)
                && CMStatus[nodeid]->hasPowerToRun(sensorid, cost)) {
                // The CM is idle, has the sensor.
                // CMStatus is updated in sreateSubTask.
                return task->createSubTask(1, CMStatus[nodeid]);
            }
        }
        it ++;
    }
    return NULL;
}

void PrioritySimpleQ::printPower() {
    cout << NOW << " PrioritySimpleQ: Print Power" << endl;
    int count = 0;
    vector<int> imf = imfCalculator->getOrderingList();
    vector<int>::iterator imfit;
    for (imfit = imf.begin(); imfit != imf.end(); imfit ++) {
        int nodeid = *imfit;
        if (CMStatus[nodeid]->getTask() == NULL) {
            cout << "#" << nodeid << ": NULL. P=" << CMStatus[nodeid]->getPower() << " ";
        }
        else if (CMStatus[nodeid]->getTask()->realTime) {
            cout << "#" << nodeid << ": RT.   P=" << CMStatus[nodeid]->getPower() << " ";
        }
        else {
            cout << "#" << nodeid << ": NRT.  P=" << CMStatus[nodeid]->getPower() << " ";
        }
        if (count % 10 == 9) {
            cout << endl;
        }
        count ++;
    }
}

int PrioritySimpleQ::assignNodeForRT(
    int sid, double cost, vector<int> * imf) {
#ifdef DEBUG
    cout << "assignNodeForRT: " << sid << "   ";
#endif
    // First find the idle nodes
    // (ones that finish at the time but are later in the queue).
    vector<int>::iterator imfit;
    for (imfit = imf->begin(); imfit != imf->end(); imfit ++) {
        int nodeid = *imfit;
        if (CMStatus[nodeid]->hasSensor(sid)
            && CMStatus[nodeid]->isAvailable()
            && CMStatus[nodeid]->hasPowerToRun(sid, cost)) {
#ifdef DEBUG
            cout << "Picked node #" << nodeid << endl;
#endif
            return nodeid;
        }
    }

    // Second interrupt the nodes with NRT tasks.
    for (imfit = imf->begin(); imfit != imf->end(); imfit ++) {
        int nodeid = *imfit;

        if (! CMStatus[nodeid]->hasSensor(sid)) {
#ifdef DEBUG
            cout << "Node#" << nodeid << " dones't have sensor: " << sid << endl;
#endif
            continue;
        }

        if (! CMStatus[nodeid]->hasPowerToRun(sid, cost)) {
#ifdef DEBUG
            cout << nodeid << " dones't have power to run: "
                 << CMStatus[nodeid]->getPower() << endl;
#endif
            continue;
        }

        if (CMStatus[nodeid]->getTask() == NULL ) {
            cerr << "Error: assignNodeForRT, node should be assigned"
                 << " in previous loop" << endl;
            continue;
        }

        SimpleSubTask * curtask = (SimpleSubTask *)(CMStatus[nodeid]->getTask());
        if (!curtask->realTime) {
            // Revert the subtask in father task.
            ((SimpleTask *)curtask->getFatherTask())->
                revertSubTask(curtask);
            return nodeid;
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
        fathertask->writeOut();
        delete fathertask;
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
