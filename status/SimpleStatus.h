/*
 * SimpleStatus.h
 *
 *  Created on: Apr 10, 2013
 *      Author: yonggang
 */

#ifndef SIMPLESTATUS_H_
#define SIMPLESTATUS_H_

#include <omnetpp.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "IStatus.h"

using namespace std;

// This is the status for one node.
class SimpleStatus : public IStatus {
protected:
    static int initId;

    int myId;
    bool sensors[MAX_SENSORS];
    double sensorCosts[MAX_SENSORS];
    double computeCap;
    double power;
    double lastPowerUpdateTime;
    ITask * currentTask;
    ITask * finishedTask;
    double period;
    double chargeRate;
    double maxPower;
    // A Status can only be created by the statusFactory method.
    SimpleStatus(double, double, double);
    void updatePower(double now);
public:
    cObject * dup();
    int getId();
    void getSensors(bool sensors[]);
    double getSensorCost(int id);
    double getComputeCap();
    double getPower();
    void assignTask(ITask * task);
    ITask * getTask();
    void taskFinish();
    double getRemainingCost();
    double predictExecutionTime(double cost);
    bool isBusy();
    bool isDead();
    bool isAvailable();
    bool hasSensor(int sid);

//    string genStatusString();
    bool parseStatusString(const string & str);
    void printInformation(); // For debugging.
    virtual ~SimpleStatus();
    friend class StatusFactory;
};

#endif /* SIMPLESTATUS_H_ */
