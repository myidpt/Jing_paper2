#ifndef __JINGQIN_CH_H_
#define __JINGQIN_CH_H_

#include <string>
#include <omnetpp.h>
#include <list>
#include "General.h"
#include "task/SimpleTask.h"
#include "iostreamer/ostreamer/CHTaskWriter.h"
#include "iostreamer/ostreamer/CMStatusWriter.h"
#include "task/TaskFactory.h"
#include "status/StatusFactory.h"
#include "scheduler/IQueue.h"
#include "scheduler/SimpleQ.h"
#include "scheduler/BalancedQ.h"
#include "scheduler/ReservedQ.h"

class CH : public cSimpleModule
{
protected:
    int numCMs;
    int numSensors;

    IStatus * CMStatus[MAX_CM];
    bool CMSensors[MAX_CM][MAX_SENSORS];
    double averageWorkloads[MAX_SENSORS];
    string algorithmName;

    bool allTraceRead;

    TaskFactory * taskFactory;
    StatusFactory * statusFactory;

    CHTaskWriter * taskWriter;
    CMStatusWriter * cmStatusWriter;

    cPacket * selfNextTaskTimer;

    cPacket * printStatusTimer;

    double printStatusStep;

    IQueue * queue; // Scheduler

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    // Distribute the initial status to every CM.
    void distributeInitialStatus();
    void getNextTask();
    void addWaitingTask();
    void processTasks();
    void processFinishedTasks(cPacket * packet);
    void printStatus();

    void sendSafe(int id, cPacket * packet);

    //bool checkResourceExhaustion();

    void finish();
};

#endif
