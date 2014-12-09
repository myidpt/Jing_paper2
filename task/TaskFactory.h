/*
 * TaskFactory.h
 *
 *  Created on: Apr 13, 2013
 *      Author: yonggang
 */

#ifndef TASKFACTORY_H_
#define TASKFACTORY_H_

#include <sstream>
#include <map>
#include "General.h"
#include "iostreamer/istreamer/Inputfile.h"
#include "ITask.h"
#include "SimpleTask.h"

class TaskFactory {
protected:
    class RTTask {
    public:
        double time;
        double cost;
        int num;
        int type;

        RTTask(double t, double c, int n, int ty)
        : time(t), cost(c), num(n), type(ty) {}
        RTTask(const RTTask& other)
        : time(other.time), cost(other.cost),
          num(other.num), type(other.type) {}
        void print() {
            cout << "Time=" << time << ", cost=" << cost << ", num=" << num
                 << ", type=" << type << endl;
        }
    };
    map<double, RTTask> rtTasks;
    ITask::TaskType myTaskType;
    Inputfile * inputfile;
    SimpleTask * nextRTTask;
    SimpleTask * nextNRTTask;
    double period;
    double periods;
    map<double, RTTask>::iterator rtTaskIt;

    bool initialAverageWorkloadsAvailable;
    double initialAverageWorkloads[MAX_SENSORS];

    bool parseInitialParameters(double input[]);
    void parseRTInputFile(const string& filename);
    void getRTTask();
public:
    TaskFactory(const string & filename, ITask::TaskType type);
    TaskFactory(const string &, const string &, ITask::TaskType, double);
    bool getInitialAverageWorkloads(double workloads[]);
    bool getBucketMaxCapRatios(double ratios[]);
    ITask * createTask();
    virtual ~TaskFactory();
};

#endif /* TASKFACTORY_H_ */
