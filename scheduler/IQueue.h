/*
 * IQueue.h
 *
 *  Created on: Apr 22, 2013
 *      Author: yonggang
 */

#ifndef IQUEUE_H_
#define IQUEUE_H_

#include <list>
#include "General.h"
#include "status/IStatus.h"
#include "task/ITask.h"

#define IDLE_SIG        -1
#define IDLE_DEAD_BOUND -10
#define DEAD_SIG        -1000

class IQueue {
public:
    IQueue();
    virtual bool setNumCMs(int) = 0;
    virtual bool setNumSensors(int) = 0;
    virtual bool setCMStatus(IStatus * status[MAX_CM]) = 0;
    virtual bool setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]) = 0;
    virtual bool setAverageWorkloads(double workloads[MAX_SENSORS]) = 0;
    virtual void setPeriod(double p);
    virtual void setChargeRate(double r);
    virtual void setNRTCost(double c);
    virtual void readTaskStats(const char * filename);
    virtual bool isEmpty() = 0;

    virtual bool newArrival(ITask * task) = 0;
    virtual ITask * dispatchNext() = 0;
    virtual bool finishedTask(ITask * task) = 0;
    virtual ~IQueue();
};

#endif /* IQUEUE_H_ */
