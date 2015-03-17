/*
 * TaskWriter.cpp
 *  Writes the task finish time stamps to the output file.
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include <vector>
#include "TaskWriter.h"
#include "Outputfile.h"

TaskWriter::TaskWriter(const string & filename) {
    outputfile = new Outputfile(filename);
    outputfile->writeLine(
        "# SimpleSubTask: (Unfinished) ID atime ftime exetime sensor_id");
}

void TaskWriter::writeSimpleSubTask(SimpleSubTask * task) {
    char buff[200];
    sprintf(buff, "%d %.2lf %.2lf %.2lf %d",
            task->getId(),
            task->getArrivalTime(),
            task->getFinishTime(),
            task->getFinishTime() - task->getArrivalTime(),
            task->getSensorId());
    outputfile->writeLine(string(buff));
}

void TaskWriter::writeUnfinishedSimpleSubTask(SimpleSubTask * task) {
    char buff[200];
    sprintf(buff, "U %d %.2lf %.2lf %.2lf %d",
            task->getId(),
            task->getArrivalTime(),
            task->getFinishTime(),
            SIMTIME_DBL(simTime()) - task->getArrivalTime(),
            task->getSensorId());
    outputfile->writeLine(string(buff));
}

TaskWriter::~TaskWriter() {
    if (outputfile != NULL) {
        delete outputfile;
    }
}

