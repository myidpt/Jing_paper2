/*
 * TaskWriter.cpp
 *  Writes the task finish time stamps to the output file.
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include <vector>
#include "TaskWriter.h"
#include "Outputfile.h"
#include "task/SimpleTask.h"
#include "task/SimpleSubTask.h"

#define BUFF_SIZE 1000

TaskWriter::TaskWriter(const string & filename) {
    outputfile = new Outputfile(filename);
    outputfile->writeLine(
        "# SimpleTask: ID atime ftime exetime sensor_id R/N delay");
    outputfile->writeLine(
        "# SUB [Server ID]stime <...>");
    outputfile->writeLine(
        "# SimpleSubTask: (Unfinished) ID atime ftime exetime sensor_id");
}

void TaskWriter::writeSimpleTask(SimpleTask * task) {
    char buff[BUFF_SIZE];
    int pos = sprintf(buff, "%d %.2lf %.2lf %.2lf %d",
            task->getId(),
            task->getArrivalTime(),
            task->getFinishTime(),
            task->getFinishTime() - task->getArrivalTime(),
            task->getSensorId());
    if (task->realTime) {
        pos += sprintf(buff + pos, " R");
    }
    else {
        pos += sprintf(buff + pos, " N");
    }
    double delay =
        task->getFinishTime() - task->getArrivalTime() - task->getMaxLatency();
    if (delay < 0) {
        delay = 0;
    }
    sprintf(buff + pos, " %.3lf", delay);
    outputfile->writeLine(string(buff));
    vector<pair<int, double> > taskstats = task->getSubTaskStats();
    vector<pair<int, double> >::iterator it;
    int position = sprintf(buff, "SUB ");
    for (it = taskstats.begin(); it != taskstats.end(); it ++) {
        position +=
            sprintf(buff + position, "[%d]%.2lf,", it->first, it->second);
        if (position > BUFF_SIZE) {
            cerr << "Output buff size not enough! "
                 << position << " > " << BUFF_SIZE << endl;
        }
    }
    outputfile->writeLine(string(buff));
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

