/*
 * RTReservation.cpp
 *
 *  Created on: Apr 13, 2014
 *      Author: winday0215
 */

#include <stdio.h>
#include <assert.h>
#include <map>
#include "Inputfile.h"
#include "RTReservation.h"
#include "SimpleTask.h"
#include "status/SimpleStatus.h"
#include "scheduler/imf/IMF.h"

using namespace std;

RTReservation::RTReservation(int numcms, int numsensors, int p, double cr)
: numCMs(numcms), numSensors(numsensors), period(p), chargeRate(cr) {
    imfCalculator = new IMF(numcms, numsensors);
}

void RTReservation::setStatus(IStatus * status[]) {
    for (int i = 0; i < MAX_CM; i ++) {
        if (status[i] == NULL) {
            break;
        }
        status[i]->getSensors(CMSensors[i]);
    }
    imfCalculator->setCMSensors(CMSensors);
}

bool RTReservation::powerAchievable(double time, double power) {
    return ((time > period/2)? period/2 : time) * chargeRate > power;
}

map<double, RTReservation::RTTask *> * RTReservation::readFile(const char  * filename) {
    map<double, RTTask *> * tasks = new map<double, RTTask *>();
    Inputfile inputfile(filename);
    string line;
    while(inputfile.readNextLine(line)) {
        double time;
        int scale;
        double totalspan;
        double span;
        int type;
        sscanf(line.c_str(), "%lf %d %lf %d", &time, &scale, &totalspan, &type);
        span = totalspan / scale;
        RTTask * rtt = new RTTask(scale, span, type);
        tasks->insert(pair<double, RTTask *>(time, rtt));
    }
    return tasks;
}


void RTReservation::getWorkloads(map<double, RTTask *> * tasks, double workloads[]) {
    for (int i = 0; i < MAX_SENSORS; i ++) {
        workloads[i] = 0;
    }
    map<double, RTTask *>::iterator it;
    for (it = tasks->begin(); it != tasks->end(); it ++) {
        workloads[it->second->type] += it->second->span * it->second->scale;
    }
}

double RTReservation::constructPThreshold(
        double task_time, double task_span, double next_time, double next_th) {
    double new_th = next_th; // The power threshold with the new subtask.
    double cost;
    double idleCharge = 0;
    if (task_time >= period/2) { // At night
        cost = task_span;
        if (task_time + task_span > period) { // Some part is at next day.
            cost = period - task_time;
            if (chargeRate < 1) {
                cost += (task_time + task_span - period) * (1 - chargeRate);
            }
        }
    }
    else if (task_time < period/2 && (task_time + task_span) > period/2) {
        // day-&-night
        cost = task_span - (period/2 - task_time) * chargeRate;
    }
    else { // All at day
        cost = task_span * (1 - chargeRate);
        double chargeEnd = next_time < (period/2) ? next_time : (period/2);
        idleCharge = (chargeEnd - task_time - task_span) * chargeRate;
    }
    new_th -= idleCharge;
    if (new_th < 0 ){
        new_th = 0;
    }
    new_th += cost;
    if (powerAchievable(task_time, new_th)) {
        return new_th;
    }
    else { // We may need a new node for more power.
        return -1;
    }
}

void RTReservation::makeReservation(const char  * filename) {
    // Initialization.
    nodesPThresholds = new map<int, PThresholds *>();
    for (int i = 0; i < MAX_SENSORS; i ++) {
        allocations[i] = new map<double, Nodeset *>();
    }
    for (int i = 0; i < numCMs; i ++) {
        nodesPThresholds->insert(
            pair<int, PThresholds *>(i, new PThresholds()));
    }

    // To track the free power on each node.
    double power[MAX_CM];
    double initial_power = period / 2 * chargeRate;
    for (int i = 0; i < MAX_CM; i ++) {
        // Initially set to the periodically recharged power.
        power[i] = initial_power;
    }

    // Read the RT task file to get the RT tasks.
    map<double, RTTask *> * tasks = readFile(filename);

    // To track the RT workload for each sensor.
    double workloads[MAX_SENSORS];
    getWorkloads(tasks, workloads);

    // Iterate the RT tasks from back to front,
    // and allocate tasks to the nodes according to the imf.
    map<double, RTTask *>::reverse_iterator taskit;
    for (taskit = tasks->rbegin(); taskit != tasks->rend(); taskit ++) {
        // For all realtime tasks.
        double task_time = taskit->first;
        double task_scale = taskit->second->scale;
        double task_span = taskit->second->span;
        int task_type = taskit->second->type;

        // Set up imfCalculator with the new power and workload.
        // Set up once per RT task.
        imfCalculator->setCMPower(power);
        imfCalculator->setWorkloads(workloads);

        multimap<double, int> imf = imfCalculator->getIMF(); // Ranked imf.
        // From smallest to largest.
        multimap<double, int>::iterator imfit = imf.begin();
        for (int subtask = 0; subtask < task_scale;) {
            // For the subtasks.
            if (imfit == imf.end()) {
                cerr << "Not enough nodes for the realtime tasks!" << endl;
                printReservation();
                return;
            }
            int nodeid = imfit->second;

            if (!CMSensors[nodeid][task_type]) { // The node does not have the sensor.
                imfit ++;
                continue;
            }

            map<int, PThresholds *>::iterator nodeit = nodesPThresholds->find(nodeid);
            if (nodeit == nodesPThresholds->end()) {
                cerr << "Can't find node#" << nodeid
                     << " in nodesPThresholds." << endl;
                return;
            }
            PThresholds * eth = nodeit->second;
            double th_time = period; // The time for the threshold.
            double th_th = 0; // The power threshold.
            // Above are default values, i.e., they are equal to no threshold.
            if (eth->begin() != eth->end()) { // PThresholds is not empty.
                th_time = eth->begin()->first;
                th_th = eth->begin()->second;
                if (th_time < task_time + task_span) { // Time overlap!
                    imfit ++;
                    continue;
                }
            }
            // Calculate the PThreshold for the new subtask.
            double new_th = constructPThreshold(
                    task_time, task_span, th_time, th_th);
            if (new_th >= 0) { // This means the subtask can be assigned here.
                eth->insert(pair<double, double>(task_time, new_th));
                map<double, Nodeset *>::iterator ait =
                        allocations[task_type]->find(task_time);
                Nodeset * ns;
                if (ait != allocations[task_type]->end()) {
                    ns = ait->second;
                }
                else {
                    ns = new Nodeset();
                    allocations[task_type]->insert(
                            pair<double, Nodeset *>(task_time, ns));
                }
                ns->insert(nodeid);
                power[nodeid] -= task_span;
                subtask ++;
            }
            imfit ++;
        }
        workloads[task_type] -= task_span * task_scale;
    }
    printReservation();
}

void RTReservation::printReservation() {
    map<int, PThresholds *>::iterator itet = nodesPThresholds->begin();
    cout << "Node ethresholds:" << endl;
    for (; itet != nodesPThresholds->end(); itet ++) {
        cout << "Node#" << itet->first << ": ";
        PThresholds * eths = itet->second;
        PThresholds::iterator ethsit = eths->begin();
        for (; ethsit != eths->end(); ethsit ++) {
            cout << "[" << ethsit->first << "," << ethsit->second << "]";
        }
        cout << endl;
    }

    map<double, Nodeset *>::iterator it;
    cout << "RT task allocations:" << endl;
    for (int type = 0; type < MAX_SENSORS; type ++) {
        for (it = allocations[type]->begin();
             it != allocations[type]->end(); it ++) {
            double time = it->first;
            cout << "[" << time << "] ";
            Nodeset * ns = it->second;
            set<int>::iterator nsit;
            for (nsit = ns->begin(); nsit != ns->end(); nsit ++) {
                cout << *nsit << " ";
            }
            cout << endl;
        }
    }
}

// Compare the NRT task against the RT tasks,
// to see if the time slot has conflict or power cannot be reserved.
bool RTReservation::findViolationForNode(
        int id, double power, double now, double cost) {
    int s_periods = now / period;
    double s_offset = now - s_periods * period;
    int e_periods = (now + cost) / period;
    double e_offset = now + cost - e_periods * period;

    // Assume cost < period/2.
    if (s_offset <= period/2) { // s, M
        if (e_offset <= period/2) {
            power -= (1 - chargeRate) * (e_offset - s_offset);
        }
        else {
            power -= (1 - chargeRate) * (period/2 - s_offset)
                     + (e_offset - period/2);
        }
    }
    else { // M, s
        if (e_offset > period/2) { // M, s, e, N
            power -= e_offset - s_offset;
        }
        else { // M, s, N, e
            power -= (1 - chargeRate) * e_offset + period - s_offset;
        }
    }

    if (power < 0) { // Power is not enough!
        return true;
    }

    map<int,  PThresholds *>::iterator it = nodesPThresholds->find(id);
    if (it == nodesPThresholds->end()) { // This should not happen.
        cerr << "Cannot find node#" << id << " in nodesPThresholds." << endl;
        return false;
    }
    map<double, double> * pThresholds = it->second;
    map<double, double>::iterator ptit;
    double rtt_offset;
    double rtt_power;
    // Find time interference.
    // Don't worry about the task will be interfering with existing RT task,
    // in that case, the node is not available.
    // Worry about if an RT task will start when the NRT task is not finished.
    for (ptit = pThresholds->begin(); ptit != pThresholds->end(); ++ ptit) {
        if (ptit->first > s_offset) {
            rtt_offset = ptit->first;
            rtt_power = ptit->second;
            break;
        }
    }
    if (ptit == pThresholds->end()) {
        if (!pThresholds->empty()) {
            // The NRT task s_offet is larger than all RT tasks,
            // then verify against the first RT task.
            rtt_offset = pThresholds->begin()->first;
            rtt_power = pThresholds->begin()->second;
        }
        else { // No RT task on this node.
            return false;
        }
    }

    if (e_offset < s_offset) { // Wripped. s, (rtt,) N, (rtt,) e
        if (rtt_offset < e_offset || s_offset < rtt_offset) {
            // rt, e, n || e, n, rt -> violate by time
            return true;
        }
        // Judge by power.
        if (rtt_offset < period/2) { // s, N, e, rtt, M
            return rtt_power > (power + chargeRate * (rtt_offset - e_offset));
        }
        else { // s, N, e, M, rtt
            return rtt_power > (power + chargeRate * (period/2 - e_offset));
        }
    }
    else { // D, s, e, D.
        if (s_offset < rtt_offset && rtt_offset < e_offset) {
            // n, rt, e -> violate by time
            return true;
        }
        // Judge by power.
        if (e_offset < period/2 && rtt_offset <= period/2) {
            // D, e, rtt, M
            return rtt_power > (power + chargeRate * (rtt_offset - e_offset));
        }
        else if (e_offset < period/2 && rtt_offset > period/2) {
            // D, e, M, rtt, N
            return rtt_power > (power + chargeRate * (period/2 - e_offset));
        }
        else { // M, e, rt, N
            return rtt_power > power;
        }
    }
}

int RTReservation::assignNodeForRT(double time, int type) {
    // Index is still within the prepared range, the same RT task.
    if (rtNodeAssignIndex[type] < rtNodeNum[type]) {
        return rtNodeAssignTracker[type][rtNodeAssignIndex[type]++];
    }
    // Index is out of the prepared range, it's the next RT task.
    double phase_time = time - (int)(time / period) * period;
    map<double, Nodeset *>::iterator ait = allocations[type]->find(phase_time);
    if (ait == allocations[type]->end()) {
        cerr << "Cannot find Time " << phase_time << " in RT allocations." << endl;
        return -1;
    }
    // Copy all server IDs from allocations to rtNodeAssignTracker.
    set<int>::iterator it;
    int i = 0;
    for (it = ait->second->begin(); it != ait->second->end(); it ++) {
        rtNodeAssignTracker[type][i ++] = *it; // Note: i ++
    }
    rtNodeNum[type] = i;
    // Point to the next server ID (Index 0 is used by this call).
    rtNodeAssignIndex[type] = 1;
    return rtNodeAssignTracker[type][0]; // Return the first server ID.
}

RTReservation::~RTReservation() {
}

