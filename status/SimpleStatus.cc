/*
 * SimpleStatus.cpp
 *
 *  Created on: Apr 10, 2013
 *      Author: yonggang
 */

#define CHECK_POWER \
    do {if (power < 0) cerr << "power is below 0" << endl; \
    else if (power > maxPower) power = maxPower; break;} while(true);

#include <assert.h>
#include "SimpleStatus.h"
#include "SimpleSubTask.h"

int SimpleStatus::initId = 0;

SimpleStatus::SimpleStatus(double p, double cr, double maxp)
:myId(initId ++), computeCap(-1), power(-1), lastPowerUpdateTime(0),
 currentTask(NULL), period(p), chargeRate(cr), maxPower(maxp) {
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

        if(l_offset < period/2) { // In sunlight.
            if(n_offset < period/2) { // In sunlight.
                day_secs = n_offset - l_offset + more_periods*period/2;
            }
            else {
                day_secs = period/2 - l_offset + more_periods*period/2;
            }
        }
        else {
            if(n_offset < period/2) {
                day_secs = n_offset + (more_periods - 1)*period/2;
            }
            else {
                day_secs = more_periods*period/2;
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

        //cout << "n=" << night_dis_rate << ", d=" << day_dis_rate << endl;
        int l_periods = lastPowerUpdateTime / period;
        double l_offset = lastPowerUpdateTime - l_periods * period;
        int n_periods = now / period;
        double n_offset = now - n_periods * period;

        //cout << "l_p=" << l_periods << ", l_o=" << l_offset
        //     << ", n_p=" << n_periods << ", n_o=" << n_offset << endl;
        if(n_offset <= period/2) {
            if(l_offset > n_offset) { // l_offset is at night.
                power -= night_dis_rate * (period - l_offset);
                CHECK_POWER
                power -= day_dis_rate * n_offset;
                CHECK_POWER
            }
            else {
                power -= day_dis_rate * (n_offset - l_offset);
                CHECK_POWER
            }
        }
        else {
            if(l_offset < period/2) {
                power -= day_dis_rate * (period/2 - l_offset);
                CHECK_POWER
                power -= night_dis_rate * (n_offset - period/2);
                CHECK_POWER
            }
            else {
                power -= night_dis_rate * (n_offset - l_offset);
                CHECK_POWER
            }
        }
    }
    lastPowerUpdateTime = now;
}

// Update the power at this time point
double SimpleStatus::getPower() {
    updatePower(SIMTIME_DBL(simTime()));
    return power;
}

void SimpleStatus::assignTask(ITask * task) {
    updatePower(SIMTIME_DBL(simTime()));
    currentTask = task;
    idleTime = SIMTIME_DBL(simTime()) + task->getComputeCost() / computeCap;
}

void SimpleStatus::taskFinish() {
    updatePower(SIMTIME_DBL(simTime()));
    currentTask = NULL;
    idleTime = 0;
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
    else {
        return false;
    }
}

double SimpleStatus::getRemainingCost() {
    if (currentTask == NULL) {
        cerr << "Error: getRemainingCost" << endl;
        return 0;
    }
    return currentTask->getComputeCost()
        - ((SIMTIME_DBL(simTime()) - currentTask->getServiceTime()) * computeCap);
}

double SimpleStatus::predictExecutionTime(double cost) {
    return cost / computeCap;
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

void SimpleStatus::test() {
    cout << "Testing SimpleStatus.-----------------------" << endl;
    period = 4000;
    chargeRate = 0.5;
    maxPower = 500;
    power = maxPower;
    sensorCosts[1] = 1;
    updatePower(0);
    assert(power == maxPower);
    updatePower(100);
    assert(power == maxPower);
    assert(lastPowerUpdateTime == 100);

    SimpleSubTask * st1 = new SimpleSubTask(NULL, 1, NULL);
    st1->sensorId = 1;

    currentTask = st1; // Assign task at 100.
    updatePower(400);
    currentTask = NULL; // Task finish.
    assert(power == 350);
    updatePower(700);
    assert(power == 500); // Charged.
    updatePower(1000);
    assert(power == maxPower);
    updatePower(1800);
    currentTask = st1; // Assign task at 1800.
    updatePower(2100);
    assert(power == 300);
    currentTask = NULL; // Task finish.
    updatePower(3000);
    assert(power == 300);
    updatePower(4200);
    assert(power == 400); // Charged 100.
    currentTask = st1; // Assign task at 100.
    updatePower(4400);
    assert(power == 300); // Discharged 100.
    updatePower(4600);
    assert(power == 200); // Discharged 100.
    currentTask = NULL;
    updatePower(7800);
    assert(power == 500); // Charged to full.
    currentTask = st1; // Assign task at 7800.
    updatePower(8200);
    assert(power == 200); // Discharged 300.
    currentTask = NULL;

    cout << "At 3000, power=" << power << endl;
    cout << "Testing SimpleStatus done.-----------------------" << endl;
    while(true);
}

SimpleStatus::~SimpleStatus() {
}

