/*
 * BalancedQ.h
 *
 *  Created on: Apr 23, 2013
 *      Author: yonggang
 */

#ifndef BalancedQ_H_
#define BalancedQ_H_

#define ADJUSTMENT 1

#include <list>
#include <math.h>
#include <omnetpp.h>
#include "General.h"
#include "scheduler/IQueue.h"
#include "status/IStatus.h"
#include "task/ITask.h"
#include "task/SimpleTask.h"

#define MINIMAL         0.00001
#define EQUAL(A,B)      ((((A)-(B) < MINIMAL) && ((A)-(B) >= 0)) || (((B)-(A) < MINIMAL) && ((B)-(A) >= 0)))

class BalancedQ : public IQueue {
protected:
    int numCMs;
    int numSensors;
    IStatus * CMStatus[MAX_CM];
    bool CMSensors[MAX_CM][MAX_SENSORS];
    double averageWorkloads[MAX_SENSORS];
    double CMIdleTime[MAX_CM]; // -1000: dead, -1: idle, positive: busy.
    double setImportantFactorTime;

    list<ITask *> * taskQ;

    double sensorTimeShare[MAX_CM][MAX_SENSORS];
    void setSensorTimeShare();

    double resourceQuantity[MAX_SENSORS]; // The resource quantity for each sensor.
    void setResourceQuantity();

    double WRRatio[MAX_SENSORS];
    double NWRRatio[MAX_SENSORS];
    void setWRRatios();

    double importanceFactor[MAX_CM];
    void setImportanceFactor();

    double averageCompCap[MAX_SENSORS];
    void setAverageCompCap();

    int ifOrders[MAX_CM];
    void orderImportanceFactors();

    int getParaDegree(ITask * task);

    bool laggedBehind(ITask * task);

    void setCMDead(int cmid);

    void rankImportanceFactorByEnergy(int, int);
public:
    BalancedQ(int, int);

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

    virtual ~BalancedQ();
};

#endif /* BalancedQ_H_ */
