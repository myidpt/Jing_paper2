/*
 * Randomly rank the nodes.
 */

#ifndef RANDOM_H_
#define RANDOM_H_

#include "scheduler/ordering/Ordering.h"

class Random : public Ordering {
    static bool initiated;
    static void init();
public:
    Random(int numcms, int numsensors);
    virtual vector<int> getOrderingList();
    virtual ~Random();
};

#endif /* RANDOM_H_ */
