/*
 * Author: Yonggang Liu <myidpt@gmail.com>
 * Date: Dec.29, 2014.
 * IMF is the importance factor.
 * It is calculated for each node, to reflect the importance of the nodes
 * with respect to the sensors it has.
 * Higher IMF means the node holds more "rare" sensors.
 * A sensor is more "rare" if more workloads require the sensor,
 * and fewer nodes have it.
 *
 * Before calculating the importance factors,
 * you need to set the CM power (setCMPower), CM sensors (setCMSensors)
 * and set the workloads (setWorkloads).
 * getIMF calculates the importance factors for the nodes
 * (ordered from low to high according to the importance factor).
 * This class records the CM power/sensors and workloads,
 * therefore, getIMF can be called multiple times with modifications
 * on each of the configurations.
 */

#ifndef IMF_H_
#define IMF_H_

#include <map>
#include "General.h"

using namespace std;

class IMF {
protected:
    int numCMs;
    int numSensors;
    double CMPower[MAX_CM];
    bool CMSensors[MAX_CM][MAX_SENSORS];
    double workloads[MAX_SENSORS];
public:
    static void printIMF(multimap<double, int> imf);
    IMF(int numcms, int numsensors);
    // You need to set power, sensors and workload to get PWR.
    void setCMPower(double p[MAX_CM]);
    void setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]);
    void setWorkloads(double workloads[MAX_SENSORS]);
    multimap<double, int> getIMF();
    virtual ~IMF();
};

#endif /* PWR_H_ */
