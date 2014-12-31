/*
 * CHTaskWriter.h
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#ifndef CHTASKWRITER_H_
#define CHTASKWRITER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "Outputfile.h"
#include "task/SimpleTask.h"

using namespace std;

class CHTaskWriter {
protected:
    Outputfile * outputfile;
public:
    CHTaskWriter(const string & filename);
    bool writeSimpleTask(SimpleTask * task);
    virtual ~CHTaskWriter();
};

#endif /* CHTASKWRITER_H_ */
