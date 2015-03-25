/*
 * Randomly rank the nodes.
 */

#include <stdlib.h>
#include <time.h>
#include "Random.h"
//#define DEBUG

bool Random::initiated = false;

void Random::init() {
    srand(1001);
    initiated = true;
}

Random::Random(int numcms, int numsensors)
: Ordering(numcms, numsensors) {
    if (! initiated) {
        init();
    }
}

vector<int> Random::getOrderingList() {
    int order[MAX_CM];
    for (int i = 0; i < numCMs; i ++) {
        order[i] = i;
    }

    // Shuffle
    for (int i = 0; i < numCMs; i ++) {
        int randind = ((double)rand()) / RAND_MAX * numCMs;
        int tmp = order[i];
        order[i] = order[randind];
        order[randind] = tmp;
    }
    vector<int> ret;
    for (int i = 0; i < numCMs; i ++) {
        ret.push_back(order[i]);
    }
#ifdef DEBUG
    printOrdering(ret);
#endif
    return ret;
}


Random::~Random() {
}

