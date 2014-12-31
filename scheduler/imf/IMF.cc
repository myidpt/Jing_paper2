#include <iostream>
#include <map>
#include <stdio.h>
#include "IMF.h"

using namespace std;

IMF::IMF(int numcms, int numsensors) {
    if (MAX_CM < numCMs) {
        numcms = MAX_CM;
        cerr << "MAX_CM: " << MAX_CM << " is smaller than numCM: "
             << numCMs << endl;
    }
    numCMs = numcms;

    if (MAX_SENSORS < numsensors) {
        numsensors = MAX_SENSORS;
        cerr << "MAX_SENSORS: " << MAX_SENSORS
             << " is smaller than numSensros: " << numsensors << endl;
    }
    numSensors = numsensors;
}

void IMF::setCMPower(double p[MAX_CM]) {
    for (int i = 0; i < numCMs; i ++) {
        CMPower[i] = p[i];
    }
}

void IMF::setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]) {
    for (int i = 0; i < numCMs; i ++) {
        for (int j =0; j < numSensors; j ++) {
            CMSensors[i][j] = sensors[i][j];
        }
    }
}

/*
 * Set the average workload for each sensor.
 */
void IMF::setWorkloads(double wl[MAX_SENSORS]) {
    for (int i = 0; i < numSensors; i ++) {
        workloads[i] = wl[i];
    }
}


multimap<double, int> IMF::getIMF() {
    // Get W_j/N_j.
    // IMF is initially set to workloads/count.
    double pwr[MAX_SENSORS];
    double sensorTimeShare[MAX_CM][MAX_SENSORS];
    double resourceQuantity[MAX_SENSORS];
    int count = 0;
    for (int i = 0; i < numSensors; i ++) {
        count = 0;
        for (int j = 0; j < numCMs; j ++) {
            if (CMSensors[j][i]) {
                count ++;
            }
        }
        if (count != 0) {
            pwr[i] = workloads[i] / count; // For each resource
        }
        else {
            pwr[i] = 0;
        }
    }

    for (int cm = 0; cm < numCMs; cm ++) {
        double cmtotalwl = 0;
        for (int s = 0; s < numSensors; s ++) {
            if (CMSensors[cm][s]) {
                cmtotalwl += pwr[s];
            }
        }
        for (int s = 0; s < numSensors; s ++) {
            if (CMSensors[cm][s]) {
                if (cmtotalwl == 0) {
                    sensorTimeShare[cm][s] = 0;
                }
                else {
                    sensorTimeShare[cm][s] = pwr[s] / cmtotalwl;
                }
            }
        }
    }

    double min = 10000000;
    for (int s = 0; s < numSensors; s ++) {
        resourceQuantity[s] = 0;
        for (int cm = 0; cm < numCMs; cm ++) {
            if (CMSensors[cm][s]) {
                resourceQuantity[s] += sensorTimeShare[cm][s] * CMPower[cm];
            }
        }
        if (resourceQuantity[s] != 0) {
            pwr[s] = workloads[s] / resourceQuantity[s];
            if (pwr[s] < min) {
                min = pwr[s];
            }
        }
        else {
            pwr[s] = 0;
        }
    }
    min /= 2; // MIN is set to 2.

    for (int s = 0; s < numSensors; s ++) {
        if (pwr[s] != 0) {
            pwr[s] = pwr[s] / min;
        }
        else {
            pwr[s] = 1; // Set to min.
        }
    }

    double imf[MAX_CM];
    for (int cm = 0; cm < numCMs; cm ++) {
        imf[cm] = 1;
        for (int s = 0; s < numSensors; s ++) {
            if (CMSensors[cm][s]) {
                imf[cm] *= pwr[s];
            }
        }
    }

    multimap<double, int> ret;
    for (int s = 0; s < numCMs; s ++) {
        ret.insert(pair<double, int>(imf[s], s));
    }
    return ret;
}

IMF::~IMF() {
}

