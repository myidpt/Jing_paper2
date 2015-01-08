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
    outputfile->writeLine("#ID arrival_time finish_time execution_time sensor_id");
}

bool TaskWriter::writeSimpleTask(SimpleTask * task) {
    char buff[BUFF_SIZE];
    int pos = sprintf(buff, "%d %.2lf %.2lf %.2lf %d",
            task->getId(),
            task->getArrivalTime(),
            task->getFinishTime(),
            task->getFinishTime() - task->getArrivalTime(),
            task->getSensorId());
    if (task->realTime) {
        sprintf(buff + pos, " R");
    }
    else {
        sprintf(buff + pos, " N");
    }
    outputfile->writeLine(string(buff));
    vector<pair<int, double> > taskstats = task->getSubTaskStats();
    vector<pair<int, double> >::iterator it;
    int position = sprintf(buff, "SUB ");
    for (it = taskstats.begin(); it != taskstats.end(); it ++) {
        position += sprintf(buff + position, "[%d]%.2lf,", it->first, it->second);
        if (position > BUFF_SIZE) {
            cerr << "Output buff size not enough! "
                 << position << " > " << BUFF_SIZE << endl;
        }
    }
    outputfile->writeLine(string(buff));
    return true;
}

bool TaskWriter::writeSimpleSubTask(SimpleSubTask * task) {
    char buff[200];
    sprintf(buff, "%d %.2lf %.2lf %.2lf %d",
            task->getId(),
            task->getArrivalTime(),
            task->getFinishTime(),
            task->getFinishTime() - task->getArrivalTime(),
            task->getSensorId());
    outputfile->writeLine(string(buff));
    return true;
}

TaskWriter::~TaskWriter() {
    if (outputfile != NULL) {
        delete outputfile;
    }
}

