#include <iostream>
#include <vector>
#include <stdio.h>
#include "Ordering.h"

using namespace std;

void Ordering::printOrdering(vector<int> imf) {
    vector<int>::iterator imfit = imf.begin();
    cout << "Print Ordering:";
    int i = 0;
    for (;imfit != imf.end(); imfit ++) {
        if (i % 10 == 0) {
            cout << endl;
        }
        cout << *imfit << ", ";
        i ++;
    }
    cout << endl;
}

Ordering::Ordering(int numcms, int numsensors) {
    if (MAX_CM < numcms) {
        numCMs = MAX_CM;
        cerr << "Ordering: MAX_CM: " << MAX_CM << " is smaller than numCM: "
             << numCMs << endl;
    }
    numCMs = numcms;

    if (MAX_SENSORS < numsensors) {
        numsensors = MAX_SENSORS;
        cerr << "Ordering: MAX_SENSORS: " << MAX_SENSORS
             << " is smaller than numSensros: " << numsensors << endl;
    }
    numSensors = numsensors;
}

void Ordering::setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]) {
    return; // Do nothing.
}

void Ordering::setCMPower(double p[MAX_CM]) {
    return; // Do nothing.
}

void Ordering::setWorkloads(double workloads[MAX_SENSORS]) {
    return; // Do nothing.
}

Ordering::~Ordering() {
}

