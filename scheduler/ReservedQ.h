/*
 * ReservedQ.h
 *  IMF + reservations for RT tasks.
 *  Created on: Apr 23, 2013
 *      Author: yonggang
 */

#ifndef ReservedQ_H_
#define ReservedQ_H_

#define ADJUSTMENT 1

#include <list>
#include <math.h>
#include <omnetpp.h>
#include "General.h"
#include "scheduler/realtimestats/RTReservation.h"
#include "scheduler/IQueue.h"
#include "scheduler/ordering/IMF.h"
#include "status/IStatus.h"
#include "task/ITask.h"
#include "task/SimpleTask.h"

#define MINIMAL         0.00001
#define EQUAL(A,B)      ((((A)-(B) < MINIMAL) && ((A)-(B) >= 0)) || (((B)-(A) < MINIMAL) && ((B)-(A) >= 0)))

class ReservedQ : public IQueue {
protected:
    int numCMs;
    int numSensors;
    IStatus * CMStatus[MAX_CM];
    double averageWorkloads[MAX_SENSORS];
    RTReservation * rtReserv;

    list<ITask *> * rtTaskQ;
    list<ITask *> * nrtTaskQ;

    double period;
    double chargeRate;

    Ordering * imfCalculator;

public:
    ReservedQ(int, int, double, double);

    bool setNumCMs(int);
    bool setNumSensors(int);
    bool setCMStatus(IStatus * status[MAX_CM]);
    bool setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]);
    bool setAverageWorkloads(double workloads[MAX_SENSORS]);
    void readTaskStats(const char  * filename);

    bool isEmpty();
    bool checkExhaustion();

    bool newArrival(ITask * task);
    ITask * dispatchNext();
    bool finishedTask(ITask * task);

    virtual ~ReservedQ();
};

#endif /* ReservedQ_H_ */
