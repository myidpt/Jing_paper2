/*
 * RTReservation.h
 *
 *  Created on: Apr 13, 2014
 *      Author: winday0215
 */

#ifndef RTNODES_H_
#define RTNODES_H_

#include <map>
#include <set>
#include "General.h"

using namespace std;

class RTReservation {
public:
    typedef set<int> Nodeset;
    typedef map<double, double> EThresholds;
    typedef struct RTTask {
        int scale;
        double span;
        int type;
        RTTask(int s, double sp, int t)
        : scale(s), span(sp), type(t){}
    } RTTask;
protected:
    int totalNodes;
    double period;
    double chargeRate;
    // To find the allocated nodes for the RT tasks.
    map<double, Nodeset *>* allocations[MAX_SENSORS];
    // For evaluating the energy.
    map<int, EThresholds *>* nodesEThresholds;

    // For tracking the allocation of nodes to RT tasks.
    int rtNodeAssignTracker[MAX_SENSORS][MAX_CM];
    int rtNodeAssignIndex[MAX_SENSORS];
    int rtNodeNum[MAX_SENSORS];

    inline bool energyAchievable(double time, double energy);
    inline map<double, RTTask *> * readFile(const char  * filename);
    inline double constructEThreshold(double, double, double, double);
    // Validate whether the energy is achievable if charged from beginning.
public:
    RTReservation(int tn, int p, double cr);
    void constructNodes(const char  * filename);
    void constructNodes2(const char  * filename);
    bool findViolationForNode(int i, double e, double now, double cost);
    int assignNodeForRT(double time, int type);
    virtual ~RTReservation();
};

#endif /* RTNODES_H_ */
