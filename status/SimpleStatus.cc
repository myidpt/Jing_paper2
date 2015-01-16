/*
 * SimpleStatus.cpp
 *
 *  Created on: Apr 10, 2013
 *      Author: yonggang
 */

#include <assert.h>
#include "SimpleStatus.h"
#include "SimpleSubTask.h"

#define NOW SIMTIME_DBL(simTime())
#define CHECK_POWER(id) \
    do {if (power < 0) cerr << "Status#" << (id) << ": Power is below 0" << endl; \
    else if (power > maxPower) power = maxPower;} while(false);

#define TEST_POWER_RETURN \
    do {if (test_power < 0) return false; \
    else if (test_power > maxPower) test_power = maxPower;} while(false);

int SimpleStatus::initId = 0;

SimpleStatus::SimpleStatus(double p, double cr, double maxp)
:myId(initId ++), computeCap(-1), power(-1), lastPowerUpdateTime(0),
 currentTask(NULL), finishedTask(NULL), period(p), chargeRate(cr),
 maxPower(maxp) {
    for (int i = 0; i < MAX_SENSORS; i ++) {
        sensors[i] = false;
        sensorCosts[i] = -1;
    }
}

int SimpleStatus::getId() {
    return myId;
}

void SimpleStatus::getSensors(bool sensors[]) {
    for (int i = 0; i < MAX_SENSORS; i ++) {
        sensors[i] = this->sensors[i];
    }
}

double SimpleStatus::getSensorCost(int id) {
    return sensorCosts[id];
}

double SimpleStatus::getComputeCap() {
    return computeCap;
}

void SimpleStatus::updatePower(double now) {
    if (power == maxPower && currentTask == NULL) {
        lastPowerUpdateTime = now;
        return;
    }
    if (currentTask == NULL) {
        double day_secs = 0;
        int l_periods = lastPowerUpdateTime / period;
        double l_offset = lastPowerUpdateTime - l_periods * period;
        int n_periods = now / period;
        double n_offset = now - n_periods * period;
        int more_periods = n_periods - l_periods;

        if (l_offset < period/2) { // In sunlight.
            if (n_offset < period/2) { // In sunlight.
                day_secs = n_offset - l_offset + more_periods * period/2;
            }
            else {
                day_secs = period/2 - l_offset + more_periods * period/2;
            }
        }
        else { // l_offset >= period/2
            if (n_offset < period/2) {
                day_secs = n_offset + (more_periods - 1) * period/2;
            }
            else {
                day_secs = more_periods * period/2;
            }
        }
        power += day_secs * chargeRate;
        if (power > maxPower) {
            power = maxPower;
        }
    }
    else { // Assume a subtask's execution time never exceeds 1/2 period.
        double night_dis_rate = sensorCosts[currentTask->getSensorId()];
        double day_dis_rate = night_dis_rate - chargeRate;

        int l_periods = lastPowerUpdateTime / period;
        double l_offset = lastPowerUpdateTime - l_periods * period;
        int n_periods = now / period;
        double n_offset = now - n_periods * period;

        if(n_offset <= period/2) {
            if(l_offset > n_offset) { // l_offset is at night.
                power -= night_dis_rate * (period - l_offset);
                CHECK_POWER(myId)
                power -= day_dis_rate * n_offset;
                CHECK_POWER(myId)
            }
            else {
                power -= day_dis_rate * (n_offset - l_offset);
                CHECK_POWER(myId)
            }
        }
        else {
            if(l_offset < period/2) {
                power -= day_dis_rate * (period/2 - l_offset);
                CHECK_POWER(myId)
                power -= night_dis_rate * (n_offset - period/2);
                CHECK_POWER(myId)
            }
            else {
                power -= night_dis_rate * (n_offset - l_offset);
                CHECK_POWER(myId)
            }
        }
    }
    lastPowerUpdateTime = now;
}

// Update the power at this time point
double SimpleStatus::getPower() {
    updatePower(NOW);
    return power;
}

void SimpleStatus::assignTask(ITask * task) {
    updatePower(NOW);
    if (currentTask != NULL) {
        if (currentTask->getFinishTime() == NOW) {
            cout << "RT Task queued on status." << endl;
            finishedTask = task;
        }
        else if (currentTask->realTime) {
            cerr << NOW << " ERROR: RT Task#" << task->getId()
                 << " execution interrupted: Task will finish at "
                 << task->getFinishTime() << endl;
        }
        // The current task is wiped out from status if it is not finished.
    }
    currentTask = task;
}

ITask * SimpleStatus::getTask() {
    return currentTask;
}

void SimpleStatus::taskFinish() {
    updatePower(NOW);
    if (finishedTask != NULL) { // Deal with the queuing order.
        finishedTask = NULL;
        // Don't nullify currentTask, it may be assigned with new task.
    }
    else {
        currentTask = NULL;
    }
}

bool SimpleStatus::isBusy() {
    if (currentTask == NULL) {
        return false;
    }
    else {
        return true;
    }
}

bool SimpleStatus::isDead() {
    if (power <= 0) {
        return true;
    }
    else {
        return false;
    }
}

bool SimpleStatus::hasSensor(int sid) {
    return sensors[sid];
}

bool SimpleStatus::isAvailable() {
    if (power > 0 && currentTask == NULL) {
        return true;
    }
    else if (power > 0 && currentTask != NULL
        && currentTask->getFinishTime() == NOW) {
        finishedTask = currentTask;
        currentTask = NULL;
        return true;
    }
    else {
        return false;
    }
}

double SimpleStatus::getRemainingCost() {
    if (finishedTask != NULL) {
        return 0;
    }
    else if (currentTask != NULL) {
        return currentTask->getComputeCost()
            - ((NOW - currentTask->getServiceTime()) * computeCap);
    }
    else {
        cerr << "Error: getRemainingCost" << endl;
        return 0;
    }
}

double SimpleStatus::predictExecutionTime(double cost) {
    return cost / computeCap;
}

// Note: hasPowerToRun() does not check if the node already has task running.
// For NRT tasks running, RT tasks can preempt them.
bool SimpleStatus::hasPowerToRun(int sid, double cost) {
    updatePower(NOW);

    // Assume a subtask's execution time never exceeds 1/2 period.
    double night_dis_rate = sensorCosts[sid];
    double day_dis_rate = sensorCosts[sid] - chargeRate;

    int s_periods = NOW / period;
    double s_offset = NOW - s_periods * period;
    double ftime = NOW + cost / sensorCosts[sid];
    int e_periods = ftime / period;
    double e_offset = ftime - e_periods * period;

    double test_power = power;
    cout << "test_power=" << test_power << endl;
    cout << "s_p=" << s_periods << ", s_o=" << s_offset
         << ", e_p=" << e_periods << ", e_o=" << e_offset << endl;
    if(e_offset <= period/2) {
        if(s_offset > e_offset) { // s_offset is at night.
            test_power -= night_dis_rate * (period - s_offset);
            TEST_POWER_RETURN
            test_power -= day_dis_rate * e_offset;
            TEST_POWER_RETURN
        }
        else {
            test_power -= day_dis_rate * (e_offset - s_offset);
            TEST_POWER_RETURN
        }
    }
    else {
        if(s_offset < period/2) {
            test_power -= day_dis_rate * (period/2 - s_offset);
            TEST_POWER_RETURN
            test_power -= night_dis_rate * (e_offset - period/2);
            TEST_POWER_RETURN
        }
        else {
            test_power -= night_dis_rate * (e_offset - s_offset);
            TEST_POWER_RETURN
        }
    }
    return true;
}

// Incomplete
cObject * SimpleStatus::dup() {
    SimpleStatus * status = new SimpleStatus(period, chargeRate, maxPower);
    return status;
}

bool SimpleStatus::parseStatusString(const string & str) {
    string fields[4];
    int start = 0;
    int end = 0;
    for (int i = 0; i < 3; i ++) {
        end = str.find(';', start);
        if (end == (int)string::npos) { // You meet the end before expected.
            cerr << "Meet the end before expected: " << str << endl;
            return false;
        }
        fields[i] = str.substr(start, end - start);
        start = end + 1;
    }
    fields[3] = str.substr(start, string::npos);

    istringstream stream0(fields[0]);
    istringstream stream1(fields[1]);
    istringstream stream2(fields[2]);
    if(!(stream0 >> myId)) {
        cerr << "myId parsing error." << endl;
        return false;
    }
    if(!(stream1 >> computeCap)) {
        cerr << "ComputeCap parsing error." << endl;
        return false;
    }
    if(!(stream2 >> power)) {
        cerr << "Power parsing error." << endl;
        return false;
    }
    start = 0;
    end = 0;
    int colon = 0;
    string sensoridstr;
    string valuestr;
    int sensorid = 0;
    int value = 0;

    while((colon=fields[3].find(':',start)) != string::npos) {
        sensoridstr = fields[3].substr(start, colon - start);
        end = fields[3].find(',', colon);
        if (end == (int)string::npos) {
            valuestr = fields[3].substr(colon + 1, (int)string::npos);
        }
        else {
            valuestr = fields[3].substr(colon + 1, end - colon - 1);
        }
        istringstream sensoridstream(sensoridstr);
        if(!(sensoridstream >> sensorid)) {
            cerr << "Sensor Id error:" << sensoridstr << endl;
            return false;
        }
        istringstream valuestream(valuestr);
        if(!(valuestream >> value)) {
            cerr << "Value parsing error:" << valuestr << endl;
            return false;
        }
        sensors[sensorid] = true;
        sensorCosts[sensorid] = value;
        if (end == (int)string::npos) { // This is the last one.
            break;
        }
        start = end + 1;
    }

    return true;
}

void SimpleStatus::printInformation() {
}

SimpleStatus::~SimpleStatus() {
}

