/*
 * SimpleQ.h
 *
 *  Created on: Apr 22, 2013
 *      Author: yonggang
 */

#ifndef SIMPLEQ_H_
#define SIMPLEQ_H_

#define DEGREE 32

#include <list>
#include <omnetpp.h>
#include <stdlib.h>
#include <cstdlib>
#include "General.h"
#include "scheduler/IQueue.h"
#include "status/IStatus.h"
#include "task/ITask.h"
#include "task/SimpleTask.h"

class SimpleQ : public IQueue {
protected:
    int numCMs;
    int numSensors;
    IStatus * CMStatus[MAX_CM];
    bool CMSensors[MAX_CM][MAX_SENSORS];
    double averageWorkloads[MAX_SENSORS];
    double CMIdleTime[MAX_CM];
    list<SimpleTask *> * taskQ;
public:
    SimpleQ(int, int);

    bool setNumCMs(int);
    bool setNumSensors(int);
    bool setCMStatus(IStatus * status[MAX_CM]);
    bool setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]);
    bool setAverageWorkloads(double workloads[MAX_SENSORS]);
    bool isEmpty();

    bool newArrival(ITask * task);
    ITask * dispatchNext();
    bool finishedTask(ITask * task);
    void setCMDead(int cmid);

    virtual ~SimpleQ();
};

#endif /* SIMPLEQ_H_ */
