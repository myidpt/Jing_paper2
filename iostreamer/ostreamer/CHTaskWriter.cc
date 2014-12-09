/*
 * CHTaskWriter.cpp
 *  Writes the task finish time stamps to the output file.
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include "CHTaskWriter.h"

CHTaskWriter::CHTaskWriter(const string & filename) {
    outputfile = new Outputfile(filename);
}

bool CHTaskWriter::writeTask(ITask * task) {
    char buff[100];
    sprintf(buff, "%d %.2lf %.2lf %.2lf %d",
            task->getId(),
            task->getArrivalTime(),
            task->getFinishTime(),
            task->getFinishTime() - task->getArrivalTime(),
            task->getSensorId());
    string line = buff;
    outputfile->writeLine(line);
    return true;
}

CHTaskWriter::~CHTaskWriter() {
    if (outputfile != NULL) {
        delete outputfile;
    }
}

