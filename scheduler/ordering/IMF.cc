#include <iostream>
#include <map>
#include <vector>
#include <stdio.h>
#include "IMF.h"
//#define DEBUG

using namespace std;

IMF::IMF(int numcms, int numsensors) :  Ordering(numcms, numsensors) {
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

    multimap<double, int>::iterator mit;
#ifdef DEBUG
    cout << "Print the IMFs:" << endl;
    for (mit = ret.begin(); mit != ret.end(); mit ++) {
        cout << "[" << mit->first << ", " << mit->second << "] ";
    }
    cout << endl;
#endif

    return ret;
}

vector<int> IMF::getOrderingList() {
    vector<int> retlist;

    multimap<double, int> imfmap = getIMF();
    multimap<double, int>::iterator imfit = imfmap.begin();
    double oldimf = -1;
    while (imfit != imfmap.end()) {
        multimap<double, int> pmap; // map only according to power.
        while (true) {
            if (imfit == imfmap.end() || (oldimf >= 0 && imfit->first != oldimf)) {
                // Fill retlist with nodes power from high to low.
                multimap<double, int>::reverse_iterator pit = pmap.rbegin();
                for (; pit != pmap.rend(); pit ++) {
                    retlist.push_back(pit->second);
                }
                oldimf = imfit->first;
                break;
            }
            int node = imfit->second;
            pmap.insert(pair<double, int>(CMPower[node], node));
            oldimf = imfit->first;
            imfit ++;
        }
    }
#ifdef DEBUG
        printOrdering(retlist);
#endif
    return retlist;
}

IMF::~IMF() {
}

