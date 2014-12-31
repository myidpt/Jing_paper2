/*
 * CHTaskWriter.cpp
 *  Writes the task finish time stamps to the output file.
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include <vector>
#include "CHTaskWriter.h"

CHTaskWriter::CHTaskWriter(const string & filename) {
    outputfile = new Outputfile(filename);
    outputfile->writeLine("#ID arrival_time finish_time execution_time sensor_id");
}

bool CHTaskWriter::writeSimpleTask(SimpleTask * task) {
    char buff[200];
    sprintf(buff, "%d %.2lf %.2lf %.2lf %d",
            task->getId(),
            task->getArrivalTime(),
            task->getFinishTime(),
            task->getFinishTime() - task->getArrivalTime(),
            task->getSensorId());
    outputfile->writeLine(string(buff));
    vector<pair<int, double> > taskstats = task->getSubTaskStats();
    vector<pair<int, double> >::iterator it;
    int position = sprintf(buff, "SUB ");
    for (it = taskstats.begin(); it != taskstats.end(); it ++) {
        position += sprintf(buff + position, "[%d]%.2lf,", it->first, it->second);
    }
    outputfile->writeLine(string(buff));
    return true;
}

CHTaskWriter::~CHTaskWriter() {
    if (outputfile != NULL) {
        delete outputfile;
    }
}

