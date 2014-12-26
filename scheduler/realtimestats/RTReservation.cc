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

using namespace std;

RTReservation::RTReservation(int tn, int p, double cr)
: totalNodes(tn), period(p), chargeRate(cr) {}

bool RTReservation::energyAchievable(double time, double energy) {
    return ((time > period/2)? period/2 : time) * chargeRate > energy;
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

double RTReservation::constructEThreshold(
        double task_time, double task_span, double next_time, double next_th) {
    double new_th = next_th; // The energy threshold with the new subtask.
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
    else if (task_time < period/2 && (task_time + task_span) > period/2) { // day-&-night
        cost = task_span - (period/2 - task_time) * chargeRate;
    }
    else { // All at day
        cost = task_span * (1 - chargeRate);
        idleCharge = (next_time - task_time - task_span) * chargeRate;
    }
    new_th -= idleCharge;
    if (new_th < 0 ){
        new_th = 0;
    }
    new_th += cost;
    if (energyAchievable(task_time, new_th)) {
        return new_th;
    }
    else { // We may need a new node for more energy.
        return -1;
    }
}

void RTReservation::constructNodes(const char  * filename) {
    nodesEThresholds = new map<int, EThresholds *>();
    for (int i = 0; i < MAX_SENSORS; i ++) {
        allocations[i] = new map<double, Nodeset *>();
    }
    for (int i = 0; i < totalNodes; i ++) { // Initialization.
        EThresholds * et = new EThresholds();
        nodesEThresholds->insert(pair<int, EThresholds *>(i, et));
    }
    map<double, RTTask *> * tasks = readFile(filename);
    map<double, RTTask *>::reverse_iterator taskit;
    for (taskit = tasks->rbegin(); taskit != tasks->rend(); taskit ++) {
        // For all realtime tasks.
        double task_time = taskit->first;
        double task_scale = taskit->second->scale;
        double task_span = taskit->second->span;
        int task_type = taskit->second->type;
        fflush(stdout);
        for (int subtask = 0; subtask < task_scale; subtask ++) {
            // For the subtasks.
            map<int, EThresholds *>::iterator nodeit;
            for (nodeit = nodesEThresholds->begin();
                 nodeit != nodesEThresholds->end(); nodeit ++) {
                // For all the nodes.
                int nodeid = nodeit->first;
                EThresholds * eth = nodeit->second;
                double th_time = period; // The time for the threshold.
                double th_th = 0; // The energy threshold.
                // Above are default values, i.e., they are equal to no threshold.
                if (eth->begin() != eth->end()) { // EThresholds is not empty.
                    th_time = eth->begin()->first;
                    th_th = eth->begin()->second;
                    if (th_time < task_time + task_span) { // Time overlap!
                        continue;
                    }
                }
                // Calculate the EThreshold for the new subtask.
                double new_th = constructEThreshold(task_time, task_span, th_time, th_th);
                if (new_th >= 0) {
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
                    break;
                }
            }
            if (nodeit == nodesEThresholds->end()) {
                cerr << "Not enough nodes for the realtime tasks!" << endl;
                return;
            }
        }
    }
    // Print out.
    map<int, EThresholds *>::iterator itet = nodesEThresholds->begin();
    cout << "Node ethresholds:" << endl;
    for (; itet != nodesEThresholds->end(); itet ++) {
        cout << "Node#" << itet->first << ": ";
        EThresholds * eths = itet->second;
        EThresholds::iterator ethsit = eths->begin();
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
// to see if the time slot has conflict or energy cannot be reserved.
bool RTReservation::findViolationForNode(
        int id, double e, double now, double cost) {
    int s_periods = now / period;
    double s_offset = now - s_periods * period;
    int e_periods = (now + cost) / period;
    double e_offset = now + cost - e_periods * period;

    // Assume cost < period/2.
    if (s_offset <= period/2) { // s, M
        if (e_offset <= period/2) {
            e -= (1 - chargeRate) * (e_offset - s_offset);
        }
        else {
            e -= (1 - chargeRate) * (period/2 - s_offset) + (e_offset - period/2);
        }
    }
    else { // M, n
        if (e_offset > period/2) { // M, s, e, N
            e -= e_offset - s_offset;
        }
        else { // M, s, N, e
            e -= (1 - chargeRate) * e_offset + period - s_offset;
        }
    }

    map<int,  EThresholds *>::iterator it = nodesEThresholds->find(id);
    if (it == nodesEThresholds->end()) { // This should not happen.
        cerr << "Cannot find node#" << id << " in nodesEThresholds." << endl;
        return false;
    }
    map<double, double> * eThresholds = it->second;
    map<double, double>::iterator etit;
    double rtt_offset;
    double rtt_energy;
    // Find time interference.
    // Don't worry about the task will be interfering with existing RT task,
    // in that case, the node is not available.
    // Worry about if an RT task will start when the NRT task is not finished.
    for (etit = eThresholds->begin(); etit != eThresholds->end(); ++ etit) {
        if (etit->first > s_offset) {
            rtt_offset = etit->first;
            rtt_energy = etit->second;
            break;
        }
    }
    if (etit == eThresholds->end()) {
        if (!eThresholds->empty()) {
            // The NRT task s_offet is larger than all RT tasks,
            // then verify against the first RT task.
            rtt_offset = eThresholds->begin()->first;
            rtt_energy = eThresholds->begin()->second;
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
        // Judge by energy.
        if (rtt_offset < period/2) { // s, N, e, rtt, M
            return rtt_energy > (e + chargeRate * (rtt_offset - e_offset));
        }
        else { // s, N, e, M, rtt
            return rtt_energy > (e + chargeRate * (period/2 - e_offset));
        }
    }
    else { // D, s, e, D.
        if (s_offset < rtt_offset && rtt_offset < e_offset) {
            // n, rt, e -> violate by time
            return true;
        }
        // Judge by energy.
        if (e_offset < period/2 && rtt_offset <= period/2) {
            // D, e, rtt, M
            return rtt_energy > (e + chargeRate * (rtt_offset - e_offset));
        }
        else if (e_offset < period/2 && rtt_offset > period/2) {
            // D, e, M, rtt, N
            return rtt_energy > (e + chargeRate * (period/2 - e_offset));
        }
        else { // M, e, rt, N
            return rtt_energy > e;
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

