/*
 * RTNodes.cpp
 *
 *  Created on: Apr 13, 2014
 *      Author: winday0215
 */

#include <stdio.h>
#include <assert.h>
#include <map>
#include "Inputfile.h"
#include "RTNodes.h"
#include "SimpleTask.h"

using namespace std;

void RTNodes::constructNodes(const char  * filename) {
    nodes = new map<int, Node *>();
    Inputfile inputfile(filename);
    string line;
    double time;
    int tasks;
    double duration;
    int type;
    int id = 0;
    while(inputfile.readNextLine(line)) {
        sscanf(line.c_str(), "%lf %d %lf %d", &time, &tasks, &duration, &type);
        while(tasks > id) { // Add new nodes
            Node * node = new Node(id);
            nodes->insert(pair<int, Node *>(id, node));
            id ++;
        }

        for (int i = 0; i < tasks; ++i) { // For each node, add thresholds.
            Node * node = (*(nodes->find(i))).second;
            node->thresholds.insert(pair<double, double>(time, duration));
            // Cost for the first time.
        }
    }
    for (unsigned int i = 0; i < nodes->size(); ++ i) {
        double last_time = -1;
        double last_cost = 0;
        Node * node = (*(nodes->find(i))).second;
        for (map<double, double>::reverse_iterator it = node->thresholds.rbegin();
             it != node->thresholds.rend(); ++ it) { // Do in reverse order.
            double time = (*it).first;
            double cost = (*it).second;
            if(time >= period/2) { // At night
                last_cost += cost;
                (*it).second = last_cost;
            }
            else if (time < period/2 && (time + cost) > period/2) { // day-night
                last_cost +=
                        (time + cost - period/2) + (period/2 - time) * (1 - chargeRate);
                (*it).second = last_cost;
            }
            else { // All at day
                double agg_cost = cost * (1 - chargeRate);
                last_cost = last_cost - (last_time - time - cost) * chargeRate;
                if (last_cost > 0) {
                    agg_cost += last_cost;
                }
                (*it).second = agg_cost;
                last_cost = agg_cost;
            }
            last_time = time;
        }
    }

    // For testing purpose.
    //rtNodes->testConstructNodes();
    //rtNodes->testFindViolationForNode();
}

// Compare the task against the RT tasks.
// Energy
bool RTNodes::findViolationForNode(
        int id, double e, double now, double cost) {
    int n_periods = now / period;
    double n_offset = now - n_periods * period;
    int e_periods = (now + cost) / period;
    double e_offset = now + cost - e_periods * period;

    if (n_offset < period/2) { // n, M
        if (e_offset < period/2) {
            e -= (1 - chargeRate) * (e_offset - n_offset);
        }
        else {
            e -= (1 - chargeRate) * (period/2 - n_offset) + (e_offset - period/2);
        }
    }
    else { // M, n
        if (e_offset > period/2) { // M, n, e, N
            e -= e_offset - n_offset;
        }
        else { // D, e, M, n, N
            e -= (1 - chargeRate) * e_offset + period - n_offset;
        }
    }

    map<int,  Node *>::iterator it = nodes->find(id);
    if (it == nodes->end()) { // No RT task on the node.
        if (e < 0) {
            return true;
        }
        else {
            return false;
        }
    }
    else { // There are RT tasks on the node.
        map<double, double> thresholds = it->second->thresholds;
        map<double, double>::iterator tit;
        double rt_offset;
        double rt_energy;
        for (tit = thresholds.begin(); tit != thresholds.end(); ++tit) {
            if (tit->first > n_offset) {
                rt_offset = tit->first;
                rt_energy = tit->second;
                break;
            }
        }
        if (tit == thresholds.end()) {
            rt_offset = thresholds.begin()->first;
            rt_energy = thresholds.begin()->second;
        }
        if (e_offset < n_offset) { // Wripped.
            if (rt_offset < e_offset || n_offset < rt_offset) {
                // rt, e, n || e, n, rt -> violate by time
                return true;
            }
            // Judge by energy.
            if (rt_offset < period/2) { // e, rt, M
                return rt_energy > (e + chargeRate * (rt_offset - e_offset));
            }
            else { // e, M, rt
                return rt_energy > (e + chargeRate * (period/2 - e_offset));
            }
        }
        else { // Straight.
            if (n_offset < rt_offset && rt_offset < e_offset) {
                // n, rt, e -> violate by time
                return true;
            }
            // Judge by energy.
            if (e_offset < period/2 && rt_offset <= period/2) {
                // D, e, rt, M
                return rt_energy > (e + chargeRate * (rt_offset - e_offset));
            }
            else if (e_offset < period/2 && rt_offset > period/2) {
                // D, e, M, rt, N
                return rt_energy > (e + chargeRate * (period/2 - e_offset));
            }
            else { // M, e, rt, N
                return rt_energy > e;
            }
        }
    }
}

void RTNodes::testFindViolationForNode() {
    cout << "testFindViolationForNode.-------------------" << endl;
    period = 4000;
    chargeRate = 0.4;
    constructNodes("input/test/taskstats");
    assert(findViolationForNode(5, 600, 0, 1000) == false); // OK.
    assert(findViolationForNode(5, 599, 0, 1000) == true); // Not enough energy.
    // Node 0:
    // 120(600), 280(1200), 460(1900), 300(2400), 200(3600).
    // Node 1:
    // 60(1200), 100(2400).
    //
    assert(findViolationForNode(0, 600, 500, 100) == false); // Time just OK with RT tasks.
    assert(findViolationForNode(0, 600, 500, 101) == true); // Time just not OK with RT tasks.
    // day, day
    assert(findViolationForNode(0, 400, 1000, 200) == false); // Energy just OK with RT tasks.
    assert(findViolationForNode(0, 399, 1000, 200) == true); // Energy just not OK with RT tasks.
    // day-night, night
    assert(findViolationForNode(1, 260, 1900, 200) == false); // Energy just OK with RT tasks.
    assert(findViolationForNode(1, 259, 1900, 200) == true); // Energy just not OK with RT tasks.
    // night, night
    assert(findViolationForNode(1, 200, 2100, 100) == false); // Energy just OK with RT tasks.
    assert(findViolationForNode(1, 199, 2100, 100) == true); // Energy just not OK with RT tasks.
    // night-day, day
    assert(findViolationForNode(0, 580, 3900, 700) == false); // Energy just OK with RT tasks.
    assert(findViolationForNode(0, 579, 3900, 700) == true); // Energy just not OK with RT tasks.
    // day, day, multiple
    assert(findViolationForNode(0, 400, 5000, 200) == false); // Energy just OK with RT tasks.
    assert(findViolationForNode(0, 399, 5000, 200) == true); // Energy just not OK with RT tasks.
    // day-night, night, multiple
    assert(findViolationForNode(1, 260, 9900, 200) == false); // Energy just OK with RT tasks.
    assert(findViolationForNode(1, 259, 9900, 200) == true); // Energy just not OK with RT tasks.
    cout << "testFindViolationForNode done.-------------------" << endl;
}

void RTNodes::testConstructNodes() {
    cout << "testConstructNodes.-------------------" << endl;
    period = 4000;
    chargeRate = 0.4;
    constructNodes("input/test/taskstats");
    // Verification: id = 0
    Node * node = (*(nodes->find(0))).second;
    map<double, double>::iterator it = node->thresholds.begin();
    assert((*it).first == 600);
    assert((*it).second == 120);
    ++it;
    assert((*it).first == 1200);
    assert((*it).second == 280);
    ++it;
    assert((*it).first == 1900);
    assert((*it).second == 460);
    ++it;
    assert((*it).first == 2400);
    assert((*it).second == 300);
    ++it;
    assert((*it).first == 3600);
    assert((*it).second == 200);
    // Verification: id = 1
    node = (*(nodes->find(1))).second;
    it = node->thresholds.begin();
    assert((*it).first == 1200);
    assert((*it).second == 60);
    ++it;
    assert((*it).first == 2400);
    assert((*it).second == 100);
    // Verification: id = 2
    node = (*(nodes->find(2))).second;
    it = node->thresholds.begin();
    assert((*it).first == 2400);
    assert((*it).second == 100);
    cout << "testConstructNodes done.-------------------" << endl;
}

RTNodes::~RTNodes() {
}

