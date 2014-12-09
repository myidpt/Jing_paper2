/*
 * RTNodes.h
 *
 *  Created on: Apr 13, 2014
 *      Author: winday0215
 */

#ifndef RTNODES_H_
#define RTNODES_H_

#include <map>
#include "General.h"

using namespace std;

class RTNodes {
public:
    class Node {
    public:
        int id;
        map<double, double> thresholds; // time, energy_threshold
        Node(int i) : id(i) {}
    };
protected:
    double period;
    double chargeRate;
public:

    map<int, Node *>* nodes;
    RTNodes(int p, double cr) : period(p), chargeRate(cr) {
        nodes = new map<int, Node *>();
    }
    void constructNodes(const char  * filename);
    bool findViolationForNode(int i, double e, double now, double cost);
    void testConstructNodes();
    void testFindViolationForNode();
    virtual ~RTNodes();
};

#endif /* RTNODES_H_ */
