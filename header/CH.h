#ifndef __JINGQIN_CH_H_
#define __JINGQIN_CH_H_

#include <string>
#include "General.h"
#include "task/TaskFactory.h"
#include "status/StatusFactory.h"
#include "scheduler/IQueue.h"

class CH : public cSimpleModule
{
protected:
    int numCMs;
    int numSensors;

    double period;
    IStatus * CMStatus[MAX_CM];
    bool CMSensors[MAX_CM][MAX_SENSORS];
    double averageWorkloads[MAX_SENSORS];
    string algorithmName;

    bool allTraceRead;

    TaskFactory * taskFactory;
    StatusFactory * statusFactory;

    TaskWriter * taskWriter;

    cPacket * selfNextTaskTimer;

    double tickCheckerTick;
    cPacket * selfTickChecker;

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
    void processTickChecker();

    void sendSafe(int id, cPacket * packet);

    //bool checkResourceExhaustion();

    void finish();
};

#endif
