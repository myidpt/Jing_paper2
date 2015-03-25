/*
 * Author: Yonggang Liu <myidpt@gmail.com>
 * Date: Mar. 22, 2015.
 * An interface for node orderings.
 */

#ifndef ORDERING_H_
#define ORDERING_H_

#include <vector>
#include "General.h"

using namespace std;

class Ordering {
protected:
    int numCMs;
    int numSensors;
public:
    static void printOrdering(vector<int> imf);
    Ordering(int numcms, int numsensors);
    // You need to set power, sensors and workload to get PWR.
    virtual void setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]);
    virtual void setCMPower(double p[MAX_CM]);
    virtual void setWorkloads(double workloads[MAX_SENSORS]);
    virtual vector<int> getOrderingList() = 0;
    virtual ~Ordering();
};

#endif /* ORDERING_H_ */
