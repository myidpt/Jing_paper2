#ifndef __JINGQIN_CM_H_
#define __JINGQIN_CM_H_

#include <omnetpp.h>
#include <vector>
#include "General.h"
#include "status/IStatus.h"
#include "status/SimpleStatus.h"
#include "task/ITask.h"
#include "iostreamer/ostreamer/StatusWriter.h"
#include "iostreamer/ostreamer/TaskWriter.h"


class CM : public cSimpleModule
{
protected:
    static int idInit;
    int myId;
    double period;
    cPacket * taskAtService; // To record the current task at service.
    IStatus * status;

    StatusWriter * statusWriter;
    TaskWriter * taskWriter;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    inline void processStatus(cPacket * packet);
    inline void processTask(cPacket * packet);
    inline void processFinishedTask(cPacket * packet);
    inline void processDayNightMsg(cPacket * packet);

    void sendSafe(cPacket * packet);

    void finish();
};

#endif
