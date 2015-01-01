/*
 * ITask.h
 *  This class acts as an interface for all task types.
 *  Created on: Apr 10, 2013
 *      Author: yonggang
 */

#ifndef ITASK_H_
#define ITASK_H_

#include <omnetpp.h>
#include <string>
#include <iostream>
#include "General.h"

using namespace std;

class ITask : public cOwnedObject {
public:
    enum TaskType {
        SimpleTaskType,
        SimpleSubTaskType
    };
    bool realTime;

    ITask();
    virtual int getId() = 0;
    virtual double getArrivalTime() = 0;
    virtual double getServiceTime();
    virtual void setServiceTime(double);
    virtual double getFinishTime() = 0;
    virtual void setFinishTime(double) = 0;
    virtual int getSensorId() = 0;
    virtual TaskType getTaskType() = 0;
    virtual double getInputData() = 0;
    virtual double getOutputData() = 0;
    virtual double getComputeCost() = 0;
    virtual double getRemainingCost() = 0;
    virtual double getMaxDelay() = 0;
    virtual double getRemainingTimeBeforeDeadline() = 0;
    virtual double getDeadline() = 0;
    virtual ITask * getFatherTask() = 0;
    virtual int getServerId() = 0;
    virtual void setServerId(int id) = 0;
    virtual int getTotalSubTasks();

    virtual cObject * dup() = 0;
    virtual bool setParameter(int param, double value) = 0;
    virtual double getParameter(int param) = 0;
    virtual bool parseTaskString(string & line) = 0; // Parse a line of the input file.
    virtual bool dispatched() = 0;
    virtual bool finished() = 0;
    virtual void printInformation(); // For debugging.
    virtual ~ITask();
};

#endif /* ITASK_H_ */
