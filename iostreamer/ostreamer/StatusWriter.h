/*
 * StatusWriter.h
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#ifndef STATUSWRITER_H_
#define STATUSWRITER_H_

#include <string>
#include "Outputfile.h"
#include "status/SimpleStatus.h"

using namespace std;

class StatusWriter {
protected:
    Outputfile * outputfile;
public:
    StatusWriter(const string & filename);
    void writeStatus(const string & statusstr);
    void writeStatus(IStatus * status);
    ~StatusWriter();
};

#endif /* STATUSWRITER_H_ */
