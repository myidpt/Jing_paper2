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
#include "status/SimpleStatus.h"
#include "scheduler/imf/IMF.h"

using namespace std;

class RTReservation {
public:
    typedef set<int> Nodeset;
    typedef map<double, double> PThresholds; // Power thresholds.
    typedef struct RTTask {
        int scale;
        double span;
        int type;
        RTTask(int s, double sp, int t)
        : scale(s), span(sp), type(t){}
    } RTTask;
protected:
    int numCMs;
    int numSensors;
    double period;
    double chargeRate;
    bool CMSensors[MAX_CM][MAX_SENSORS];

    // To find the allocated nodes for the RT tasks.
    map<double, Nodeset *>* allocations[MAX_SENSORS];
    // For evaluating the power.
    map<int, PThresholds *>* nodesPThresholds;

    // For tracking the allocation of nodes to RT tasks.
    int rtNodeAssignTracker[MAX_SENSORS][MAX_CM];
    int rtNodeAssignIndex[MAX_SENSORS];
    int rtNodeNum[MAX_SENSORS];

    IMF * imfCalculator;

    inline bool powerAchievable(double time, double power);
    inline map<double, RTTask *> * readFile(const char  * filename);
    inline void getWorkloads(map<double, RTTask *> * tasks, double workloads[]);
    inline double constructPThreshold(double, double, double, double);
    // Validate whether the power is achievable if charged from beginning.
    inline void printReservation();
public:
    RTReservation(int numcms, int numsensors, int p, double cr);
    void setStatus(IStatus * status[]);
    void makeReservation(const char  * filename);
    bool findViolationForNode(int i, double power, double now, double cost);
    int assignNodeForRT(double time, int type);
    void confirmAssignment(int type);
    virtual ~RTReservation();
};

#endif /* RTNODES_H_ */
