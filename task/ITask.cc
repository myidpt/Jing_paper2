/*
 * ITask.cpp
 *
 *  Created on: Apr 10, 2013
 *      Author: yonggang
 */

#include "ITask.h"

ITask::ITask() : realTime(false){
}

void ITask::printInformation() {
    cout << "printInformation() is undefined." << endl;
}

double ITask::getServiceTime() {
    cout << "setServiceTime() is undefined." << endl;
    return -1;
}

void ITask::setServiceTime(double t) {
    cout << "setServiceTime() is undefined." << endl;
}

int ITask::getTotalSubTasks() {
    cout << "getTotalSubTasks() is undefined." << endl;
}

ITask::~ITask() {
}

