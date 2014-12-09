/*
 * BalancedQ.cc
 *
 *  Created on: Apr 23, 2013
 *      Author: yonggang
 */

#include "BalancedQ.h"

BalancedQ::BalancedQ(int numcms, int numsensors) {
    // Init the queue.
    taskQ = new list<ITask *>();

    setNumCMs(numcms);
    setNumSensors(numsensors);
    // Init the idle time indicator for each CM.
    for (int i = 0; i < numCMs; i ++) {
        CMIdleTime[i] = IDLE_SIG; // Idle now.
    }
    setImportantFactorTime = -100000;
}

bool BalancedQ::setNumCMs(int numcms) {
    if (MAX_CM < numCMs) {
        cerr << "MAX_CM: " << MAX_CM << " is smaller than numCM: " << numCMs << endl;
        return false;
    }
    numCMs = numcms;
    return true;
}

bool BalancedQ::setNumSensors(int numsensors) {
    if (MAX_SENSORS < numsensors) {
        cerr << "MAX_SENSORS: " << MAX_SENSORS << " is smaller than numSensros: " << numsensors << endl;
        return false;
    }
    numSensors = numsensors;
    return true;
}


bool BalancedQ::setCMStatus(IStatus * status[MAX_CM]) {
    for (int i = 0; i < numCMs; i ++) {
        CMStatus[i] = status[i];
    }
    return true;
}

bool BalancedQ::setCMSensors(bool sensors[MAX_CM][MAX_SENSORS]) {
    for (int i = 0; i < numCMs; i ++) {
        for (int j =0; j < numSensors; j ++) {
            CMSensors[i][j] = sensors[i][j];
        }
    }
    return true;
}

/*
 * Set the average workload for each sensor.
 */
bool BalancedQ::setAverageWorkloads(double workloads[MAX_SENSORS]) {
    for (int i = 0; i < numSensors; i ++) {
        averageWorkloads[i] = workloads[i];
    }
    return true;
}

bool BalancedQ::isEmpty() {
    return taskQ->empty();
}

bool BalancedQ::newArrival(ITask * task) {
    taskQ->push_back((SimpleTask *)task);
    task->setParameter(SimpleTask::PARADEGREE, getParaDegree(task)); // Parallel degree
    return true;
}

ITask * BalancedQ::dispatchNext() {
    setImportanceFactor();

    // Test if there are idle CMs.
    bool hasidle = false;
    for (int i = 0; i < numCMs; i ++) { // For the CMs.
        if (CMIdleTime[i] < 0 && CMIdleTime[i] > IDLE_DEAD_BOUND) {
            hasidle = true;
            break;
        }
    }
    if (! hasidle) {
        return NULL;
    }

    list<ITask *>::iterator it;
    for (it = taskQ->begin(); it != taskQ->end(); it ++) { // For tasks in queue.
        SimpleTask * task = (SimpleTask *)(*it);
        if (task->dispatched()) {
            continue;
        }
        int sensorid = task->getSensorId(); // Get the required sensor ID of the task.

        int index = 0;
//        if (! laggedBehind(task)) { // Not lagged behind.
        task->setParameter(SimpleTask::PARADEGREE, getParaDegree(task));
        if (task->getParameter(SimpleTask::DEGREEFULL) > 0) {
            continue; // No need to dispatch.
        }
        for (int i = 0; i < numCMs; i ++) { // For the CMs.
            index = ifOrders[i];
            if (CMIdleTime[index] < 0 && CMIdleTime[index] > IDLE_DEAD_BOUND
                    && CMSensors[index][sensorid]) {// The CM is idle, has the sensor.
                ITask * subtask = task->createSubTask(1, CMStatus[index]);
                if (subtask == NULL) {
                    cerr << "Not right here." << endl;
                }
                subtask->setServerId(index);
                CMIdleTime[index] = SIMTIME_DBL(simTime()) +
                        subtask->getComputeCost() / CMStatus[index]->getComputeCap();
                return subtask;
            }
        }
//        }
        /*
        else { // Lagged behind - dispatch to all possible CMs.
            for (int i = 0; i < numCMs; i ++) { // For the CMs.
                index = ifOrders[i];
                if (CMIdleTime[index] < 0 && CMIdleTime[index] > IDLE_DEAD_BOUND
                        && CMSensors[index][sensorid]) { // The CM is idle, has the sensor.
                    ITask * subtask = task->createSubTask(1, CMStatus[index]);
                    if (subtask == NULL) {
                        cerr << "Not right here." << endl;
                    }
                    subtask->setServerId(index);
                    CMIdleTime[index] = SIMTIME_DBL(simTime()) +
                            subtask->getComputeCost() / CMStatus[index]->getComputeCap();
                    return subtask;
                }
            }
        }
        */
    }
    return NULL;
}

bool BalancedQ::finishedTask(ITask * task) {
    SimpleTask * fathertask = (SimpleTask *)(task->getFatherTask());
    CMIdleTime[task->getServerId()] = IDLE_SIG; // Set CM idle.
    int cmid = task->getServerId();
    if (CMStatus[cmid]->getPower() <= 0) { // This node is exhausted.
        setCMDead(cmid);
    }
    if (fathertask->setFinishedSubTask(task)) { // Remove the father task from queue.
        taskQ->remove(fathertask);
        return true;
    }
    else {
        return false;
    }
}

void BalancedQ::setCMDead(int cmid) {
    CMIdleTime[cmid] = DEAD_SIG;
    for (int i = 0; i < numSensors; i ++) {
        CMSensors[cmid][i] = false;
    }
}

/*
 * Get W_j/N_j.
 * sensorTimeShare is used by resourceQuantity.
 */
void BalancedQ::setSensorTimeShare() {
//    cout << "SensorTimeShare:" << endl;
    // perCMWorkload is the averageWorkloads/count for each resource type.
    double perCMWorkload[MAX_SENSORS];
    int count = 0;
    for (int i = 0; i < numSensors; i ++) {
        count = 0;
        for (int j = 0; j < numCMs; j ++) {
            if (CMSensors[j][i]) {
                count ++;
            }
        }
        if (count != 0) {
            perCMWorkload[i] = averageWorkloads[i] / count; // For each resource
        }
        else {
            perCMWorkload[i] = 0;
        }
    }

    // Get sensorTimeShare = one_resourceWorkload_on_CM / totelWorkload_on_CM
    for (int i = 0; i < numCMs; i ++) {
        double cmtotalwl = 0;
        for (int j = 0; j < numSensors; j ++) {
            if (CMSensors[i][j]) {
                cmtotalwl += perCMWorkload[j];
            }
        }
        for (int j = 0; j < numSensors; j ++) {
            if (CMSensors[i][j]) {
                sensorTimeShare[i][j] = perCMWorkload[j] / cmtotalwl;
            }
        }
    }
}

/*
 * resourceQuantity is used to calculate WRRatio.
 */
void BalancedQ::setResourceQuantity() {
    setSensorTimeShare();

//    cout << "ResourceQuantity:" << endl;
    for (int i = 0; i < numSensors; i ++) {
        resourceQuantity[i] = 0;
        for (int j = 0; j < numCMs; j ++) {
            if (CMSensors[j][i]) {
                resourceQuantity[i] +=
                    sensorTimeShare[j][i] * CMStatus[j]->getComputeCap() * CMStatus[j]->getPower();
            }
        }
//        cout << resourceQuantity[i] << " ";
    }
//    cout << endl;
}

/*
 * WRRatio = Workload/ResourceQuantity
 * averageWorkload is got from initial values.
 * resourceQuantity is dynamically set by setResourceQuantity().
 */
void BalancedQ::setWRRatios() {
    setResourceQuantity();

//    cout << "W-R-ratios:" << endl;
    double min = 10000000;
    for (int i = 0; i < numSensors; i ++) {
        WRRatio[i] = averageWorkloads[i] / resourceQuantity[i];
        if (WRRatio[i] < min) {
            min = WRRatio[i];
        }
    }

    for (int i = 0; i < numSensors; i ++) {
        NWRRatio[i] = WRRatio[i] / min;
    }
}

void BalancedQ::setImportanceFactor() {
    setWRRatios();

//    cout << "ImportanceFactor: " << endl;
    for (int i = 0; i < numCMs; i ++) {
        importanceFactor[i] = 1;
        for (int j = 0; j < numSensors; j ++) {
            if (CMSensors[i][j]) {
                importanceFactor[i] *= NWRRatio[j];
            }
        }
//        cout << importanceFactor[i] << " ";
    }
    orderImportanceFactors();
//    cout << endl;
}

void BalancedQ::orderImportanceFactors() {
    if (setImportantFactorTime + 5 > SIMTIME_DBL(simTime())) {
        return;
    }
    setImportantFactorTime = SIMTIME_DBL(simTime());
    int j = numCMs - 1;
    int mark[numCMs];
    for (int i = 0; i < numCMs; i ++) {
        mark[i] = 0;
    }
    while (j >= 0) {
        int max = -1;
        int maxindex = -1;
        for (int i = 0; i < numCMs; i ++) {
            if ((mark[i] == 0) && (max < importanceFactor[i])) {
                max = importanceFactor[i];
                maxindex = i;
            }
        }
        if (maxindex != -1) {
            ifOrders[j] = maxindex;
            mark[maxindex] = 1;
            j --;
        }
        else {
            break;
        }
    }

    // Rank according to the residue energy.
    double currentfactor = 0;
    int start = 0;
    for (int i = 0; i < numCMs; i ++) {
        if(EQUAL(importanceFactor[ifOrders[i]], currentfactor)) {
            continue;
        }
        else {
            if (start < i - 1) { // You have some equal factors.
                rankImportanceFactorByEnergy(start, i);
            }
            start = i;
            currentfactor = importanceFactor[ifOrders[i]];
        }
    }
    if (start < numCMs - 1) {
        rankImportanceFactorByEnergy(start, numCMs);
    }
/*
    cout << "orderImportanceFactors:" << endl;
    for (int i = 0; i < numCMs; i ++) {
        cout << ifOrders[i] << "\t";
    }
    cout << endl;

    for (int i = 0; i < numCMs; i ++) {
        cout << importanceFactor[ifOrders[i]] << "\t";
    }
    cout << endl;

    for (int i = 0; i < numCMs; i ++) {
        if (CMStatus[ifOrders[i]]->isBusy()) {
            cout << "+\t";
        }
        else {
            cout << "-\t";
        }
    }
    cout << endl;
    */
}

void BalancedQ::rankImportanceFactorByEnergy(int start, int end) {
//    cout << "Rank:" << start << "-" << end << endl;
    int tmp;
    for (int i = start; i < end; i ++) { // Insert sort.
        for (int j = i + 1; j < end; j ++) {
            if (CMStatus[ifOrders[i]]->getPower() < CMStatus[ifOrders[j]]->getPower()) {
                tmp = ifOrders[i];
                ifOrders[i] = ifOrders[j];
                ifOrders[j] = tmp;
            }
        }
    }
}

void BalancedQ::setAverageCompCap() {
    int count = 0;
    for (int i = 0; i < numSensors; i ++) {
        averageCompCap[i] = 0;
        count = 0;
        for (int j = 0; j < numCMs; j ++) {
            if (CMSensors[j][i]) {
                averageCompCap[i] += CMStatus[j]->getComputeCap();
                count ++;
            }
        }
        if (count != 0) {
            averageCompCap[i] /= count;
        }
    }
}

int BalancedQ::getParaDegree(ITask * t) {
    setAverageCompCap();
    SimpleTask * task = (SimpleTask *)t;
    double singleTime = task->getUndispatchedSubTasks() * task->getSubTaskCost() / averageCompCap[task->getSensorId()];
    double tBatch = task->getRemainingTimeBeforeDeadline();
    cout << "getRemainingTimeBeforeDeadline = " << task->getRemainingTimeBeforeDeadline();
    if (task->getConcurrency() != 0) {
        double servingwl = task->getServingWorkload();
        tBatch = task->getRemainingTimeBeforeDeadline()
            - servingwl / averageCompCap[task->getSensorId()] / task->getConcurrency();
        cout << ", getServingWorkload = " << servingwl;
        cout << ", averageCompCap = " << averageCompCap[task->getSensorId()];
        cout << ", getConcurrency = " << task->getConcurrency();
    }
    if (tBatch < task->getSubTaskCost() / averageCompCap[task->getSensorId()]) {
        return 1000;
    }
    double degree = singleTime / tBatch * ADJUSTMENT + 1;
    cout << ", singleTime = " << singleTime;
    cout << ", tBatch = " << tBatch;
    cout << ", Degree = " << degree << endl;
    return (int)degree;
}

/*
bool BalancedQ::laggedBehind(ITask * task) {
    int degree = getParaDegree(task);
    double numSubTasks = ((SimpleTask *)task)->getUnfinishedSubTasks();
    double batches = numSubTasks / degree;
    if (batches < 1) {
        batches = 1;
    }

    double timeForBatch = task->getRemainingTimeBeforeDeadline()
            - ((SimpleTask *)task)->getRemainingCost() / averageCompCap[task->getSensorId()]
            / ((SimpleTask *)task)->getConcurrency();

    double timeForOneBatch = ((SimpleTask *)task)->getSubTaskCost() / averageCompCap[task->getSensorId()];
    double timeNeeded = timeForOneBatch * batches;

    double targetBatches = timeForBatch / timeForOneBatch;

    if (targetBatches > batches) {
        cout << "Task #" << task->getId() << " is lagged behind. Degree = " << degree << endl;
        int targetdegree = targetBatches / batches * degree;
        task->setParameter(SimpleTask::PARADEGREE, targetdegree);
        return true;
    }
    return false;
}
*/

BalancedQ::~BalancedQ() {
    if (taskQ) {
        delete taskQ;
        taskQ = NULL;
    }
}


