/*
 * PrioritySimpleQ.h
 *
 *  Created on: Apr 23, 2013
 *      Author: yonggang
 */

#ifndef PRIORITYSIMPLEQ_H_
#define PRIORITYSIMPLEQ_H_

#define ADJUSTMENT 1

#include <list>
#include <math.h>
#include <omnetpp.h>
#include "General.h"
#include "scheduler/IQueue.h"
#include "scheduler/imf/IMF.h"
#include "status/IStatus.h"
#include "task/ITask.h"
#include "task/SimpleTask.h"

#define MINIMAL         0.00001
#define EQUAL(A,B)      ((((A)-(B) < MINIMAL) && ((A)-(B) >= 0))\
        || (((B)-(A) < MINIMAL) && ((B)-(A) >= 0)))

class PrioritySimpleQ : public IQueue {
protected:
    int numCMs;
    int numSensors;
    IStatus * CMStatus[MAX_CM];
    double averageWorkloads[MAX_SENSORS];
    list<ITask *> * rtTaskQ;
    list<ITask *> * nrtTaskQ;
    IMF * imfCalculator;

    int assignNodeForRT(int sid, multimap<double, int> * imfmap);

public:
    PrioritySimpleQ(int, int);

    bool setNumCMs(int);
    bool setNumSensors(int);
    bool setCMStatus(IStatus * status[MAX_CM]);
    bool setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]);
    bool setAverageWorkloads(double workloads[MAX_SENSORS]);

    bool isEmpty();
    bool checkExhaustion();

    bool newArrival(ITask * task);
    ITask * dispatchNext();
    bool finishedTask(ITask * task);

    virtual ~PrioritySimpleQ();
};

#endif /* PRIORITYSIMPLEQ_H_ */
