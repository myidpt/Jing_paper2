/*
 * TaskWriter.h
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#ifndef TASKWRITER_H_
#define TASKWRITER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "Outputfile.h"
#include "task/SimpleTask.h"
#include "task/SimpleSubTask.h"

using namespace std;

class TaskWriter {
protected:
    Outputfile * outputfile;
public:
    TaskWriter(const string & filename);
    void writeSimpleTask(SimpleTask * task);
    void writeSimpleSubTask(SimpleSubTask * task);
    void writeUnfinishedSimpleSubTask(SimpleSubTask * task);
    virtual ~TaskWriter();
};

#endif /* TASKWRITER_H_ */
