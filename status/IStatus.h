/*
 * IStatus.h
 *
 *  Created on: Apr 12, 2013
 *      Author: yonggang
 */

#ifndef ISTATUS_H_
#define ISTATUS_H_

#include <omnetpp.h>
#include <string>
#include "General.h"
#include "task/ITask.h"

using namespace std;

class IStatus : public cOwnedObject {
public:
    IStatus();
    virtual cObject * dup() = 0;
    virtual int getId() = 0;
    virtual void getSensors(bool sensors[]) = 0;
    virtual double getSensorCost(int id) = 0;
    virtual double getComputeCap() = 0;
    virtual double getPower() = 0;
    virtual void assignTask(ITask * task) = 0;
    virtual void taskFinish() = 0;
    virtual bool isBusy() = 0;
    virtual bool isDead() = 0;
    virtual bool isAvailable() = 0; // It must be free & alive
    virtual double getRemainingCost() = 0;
    virtual double predictExecutionTime(double cost) = 0;
    virtual bool hasSensor(int sid) = 0;
//    string genStatusString();
    virtual bool parseStatusString(const string & str) = 0;
    virtual void printInformation() = 0; // For debugging.
    virtual ~IStatus();
};

#endif /* ISTATUS_H_ */
