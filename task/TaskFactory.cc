/*
 * TaskFactory.cc
 *
 *  Created on: Apr 13, 2013
 *      Author: yonggang
 */

#include <map>
#include "TaskFactory.h"

using namespace std;

TaskFactory::TaskFactory(const string & filename, ITask::TaskType type) {
    inputfile = new Inputfile(filename);
    myTaskType = type;
    initialAverageWorkloadsAvailable = parseInitialParameters(initialAverageWorkloads);
    nextNRTTask = NULL;
}

TaskFactory::TaskFactory(
        const string & filename, const string & filename2, ITask::TaskType type, double p) {
    inputfile = new Inputfile(filename);
    myTaskType = type;
    initialAverageWorkloadsAvailable = parseInitialParameters(initialAverageWorkloads);
    nextNRTTask = NULL;
    parseRTInputFile(filename2);
    nextRTTask = NULL;
    period = p;
    periods = 0;
    rtTaskIt = rtTasks.begin();
}

void TaskFactory::parseRTInputFile(const string& filename) {
    Inputfile inputfile(filename);
    string line;
    double time;
    int tasks;
    double duration;
    int type;
    while(inputfile.readNextLine(line)) {
        sscanf(line.c_str(), "%lf %d %lf %d", &time, &tasks, &duration, &type);
        RTTask rtt(time, duration, tasks, type);
        rtTasks.insert(pair<double, RTTask>(time, rtt));
    }
}

bool TaskFactory::parseInitialParameters(double input[]) {
    string line;
    if (!inputfile->readNextLine(line)) {
        return false;
    }
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
        input[sensorid] = value;
        if (end == (int)string::npos) { // This is the last one.
            return true;
        }
        start = end + 1;
    }

    for (int i = 0; i < MAX_SENSORS; i ++) { // Set to be default.
        input[i] = 0;
    }

    return false;
}

void TaskFactory::getRTTask() {
    if(rtTasks.empty()) {
        cerr << "No RT tasks." << endl;
        return;
    }
    if(rtTaskIt == rtTasks.end()) { // Initial
        rtTaskIt = rtTasks.begin();
        periods += period;
    }
    RTTask task = rtTaskIt->second;
    rtTaskIt++;

    nextRTTask = new SimpleTask();
    nextRTTask->set(task.time + periods, task.num, task.type, task.cost * task.num);
    nextRTTask->realTime = true;
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
    if (nextRTTask == NULL) {
        getRTTask();
        if(nextRTTask == NULL) {
            cerr << "No RT task got." << endl;
        }
    }

    if (nextNRTTask == NULL) {
        string str;
        if (inputfile->readNextLine(str)) {
            nextNRTTask = new SimpleTask();
            if(!nextNRTTask->parseTaskString(str)) {
                delete nextNRTTask;
                nextNRTTask = NULL;
            }
        }
    }

    SimpleTask * task;
    if (nextRTTask->arrivalTime > nextNRTTask->arrivalTime) {
        task = nextNRTTask;
        nextNRTTask = NULL;
    }
    else {
        task = nextRTTask;
        nextRTTask = NULL;
    }
    cout << "Task: " << task->Id << ", arrivalTime=" << task->arrivalTime
         << ", totalSubtasks=" << task->totalSubTasks << endl;
    return task;
}

TaskFactory::~TaskFactory() {
}

