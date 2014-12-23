/*
 * TaskFactory.cc
 *
 *  Created on: Apr 13, 2013
 *      Author: yonggang
 */

#include <map>
#include "TaskFactory.h"

using namespace std;

TaskFactory::TaskFactory(const string & filename, ITask::TaskType type)
: myTaskType(type), nextNRTTask (NULL) {
    parseNRTInputFile(filename);
}

TaskFactory::TaskFactory(
        const string & filename1, const string & filename2, ITask::TaskType type, double p)
: myTaskType(type), nextNRTTask (NULL), nextRTTask(NULL), period(p), periods(0) {
    parseNRTInputFile(filename1);
    parseRTInputFile(filename2);
}

void TaskFactory::parseRTInputFile(const string& filename) {
    Inputfile rt_inputfile(filename);
    string line;
    double time;
    int tasks;
    double cost;
    int type;
    while(rt_inputfile.readNextLine(line)) {
        sscanf(line.c_str(), "%lf %d %lf %d", &time, &tasks, &cost, &type);
        RTTask rtt(time, cost, tasks, type);
        rtTasks.insert(pair<double, RTTask>(time, rtt));
    }
    rtTaskIt = rtTasks.begin();
    getRTTask();
}

void TaskFactory::parseNRTInputFile(const string & filename) {
    nrt_inputfile = new Inputfile(filename);
    string line;
    if (!nrt_inputfile->readNextLine(line)) {
        initialAverageWorkloadsAvailable = false;
        return;
    }
    getNRTTask();

    int start = 0;
    int end = 0;
    int colon = 0;
    string sensoridstr;
    string valuestr;
    int sensorid = 0;
    double value = 0;
    while((colon=line.find(':',start)) != string::npos) {
        sensoridstr = line.substr(start, colon - start);
        end = line.find(',', colon);
        if (end == (int)string::npos) {
            valuestr = line.substr(colon + 1, (int)string::npos);
        }
        else {
            valuestr = line.substr(colon + 1, end - colon - 1);
        }
        istringstream sensoridstream(sensoridstr);
        if(!(sensoridstream >> sensorid)) {
            cerr << "parseInitialParameters: Sensor Id error:" << sensoridstr << endl;
            break;
        }
        istringstream valuestream(valuestr);
        if(!(valuestream >> value)) {
            cerr << "parseInitialParameters: Value parsing error:" << valuestr << endl;
            break;
        }
        initialAverageWorkloads[sensorid] = value;
        if (end == (int)string::npos) { // This is the last one.
            initialAverageWorkloadsAvailable = true;
            return;
        }
        start = end + 1;
    }

    for (int i = 0; i < MAX_SENSORS; i ++) { // Set to be default.
        initialAverageWorkloads[i] = 0;
    }
    initialAverageWorkloadsAvailable = false;
}

void TaskFactory::getRTTask() {
    if(rtTasks.empty()) {
        cout << "No RT tasks." << endl;
        return;
    }
    if(rtTaskIt == rtTasks.end()) { // Initial
        rtTaskIt = rtTasks.begin();
        periods += period;
    }
    RTTask task = rtTaskIt->second;
    rtTaskIt++;

    nextRTTask = new SimpleTask();
    nextRTTask->realTime = true;
    nextRTTask->set(task.time + periods, task.num, task.type, task.cost);
}

void TaskFactory::getNRTTask() {
    string str;
    if (nrt_inputfile->readNextLine(str)) {
        nextNRTTask = new SimpleTask();
        nextNRTTask->realTime = false;
        if(!nextNRTTask->parseTaskString(str)) {
            delete nextNRTTask;
            nextNRTTask = NULL;
        }
    }
}

bool TaskFactory::getInitialAverageWorkloads(double workloads[]) {
    if (! initialAverageWorkloadsAvailable) {
        return false;
    }
    for (int i = 0; i < MAX_SENSORS; i ++) {
        workloads[i] = initialAverageWorkloads[i];
    }
    return true;
}

ITask * TaskFactory::createTask() {
    SimpleTask * task;
    if (nextRTTask == NULL && nextNRTTask == NULL) {
        cout << "No more tasks to dispatch." << endl;
        return NULL;
    }
    else if (nextRTTask == NULL
        || (nextNRTTask != NULL
            && nextRTTask->arrivalTime > nextNRTTask->arrivalTime)) {
        task = nextNRTTask;
        nextNRTTask = NULL;
        getNRTTask();
    }
    else {
        task = nextRTTask;
        nextRTTask = NULL;
        getRTTask();
    }
    return task;
}

TaskFactory::~TaskFactory() {
}

