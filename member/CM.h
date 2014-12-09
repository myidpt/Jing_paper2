

#ifndef __JINGQIN_CM_H_
#define __JINGQIN_CM_H_

#include <omnetpp.h>
#include "General.h"
#include "status/IStatus.h"
#include "status/SimpleStatus.h"
#include "task/ITask.h"


class CM : public cSimpleModule
{
protected:
    static int idInit;
    int myId;
    cPacket * taskAtService; // To record the current task at service.
    IStatus * status;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    inline void processStatus(cPacket * packet);
    inline void processTask(cPacket * packet);
    inline void processFinishedTask(cPacket * packet);

    void sendSafe(cPacket * packet);
};

#endif
