/*
 * Task.h
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#ifndef SIMPLETASK_H_
#define SIMPLETASK_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include "ITask.h"
#include "SimpleSubTask.h"
#include "status/IStatus.h"

using namespace std;

class SimpleTask : public ITask {
protected:
    static int rtInitId;
    int Id;
    double arrivalTime;
    double finishTime;
    int totalSubTasks;
    int sensorId;
    double inputData;
    double outputData;
    double computeCost;
    double maxDelay;
    int undispatchedSubTasks;
    int unfinishedSubTasks;
    int subId;
    int paradegree; // target concurrency
    int concurrency; // current concurrency
    list<ITask * > * subTasks;

    SimpleTask();
public:
    enum Parameter {
        TOTAL_SUBTASKS, // From input.
        INPUT_DATA, // From input.
        OUTPUT_DATA, // From input.
        COMPUTE_COST, // From input.
        MAX_DELAY, // From input.

        TOTAL_RUNTIME,
        NODES_USED,
        SUCCESS_RATE,

        CONCURRENCY,
        PARADEGREE,
        DEGREEFULL,

        SUBTASK_COST
    };
    cObject * dup();
    int getId();
    int getSensorId();
    double getArrivalTime();
    double getFinishTime();
    ITask * createSubTask(int num, IStatus * server);
    ITask * getFatherTask();
    ITask::TaskType getTaskType();
    double getInputData();
    double getOutputData();
    double getComputeCost();
    double getRemainingCost();
    double getServingWorkload();
    double getRemainingTimeBeforeDeadline();
    double getMaxDelay();
    int getServerId();
    void setServerId(int id);
    int getDegree();
    int getUnfinishedSubTasks();
    int getUndispatchedSubTasks();
    int getSubTaskCost();
    int getConcurrency();
    int getTotalSubTasks();

    bool setParameter(int param, double value);
    double getParameter(int param);
    bool setFinishedSubTask(ITask * task);
    bool parseTaskString(string & line);
    void set(double time, int num, int sid, double cc);
    bool dispatched();
    bool finished();
    void printInformation();
    virtual ~SimpleTask();
    friend class TaskFactory;
};

#endif /* SIMPLETASK_H_ */
