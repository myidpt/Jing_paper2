/*
 * StatusFactory.cc
 *
 *  Created on: Apr 12, 2013
 *      Author: yonggang
 */

#include "StatusFactory.h"

StatusFactory::StatusFactory(const string & filename, StatusType type,
                             double p, double cr, double maxp)
:myStatusType(type), period(p), chargeRate(cr), maxPower(maxp),
 inputfile(new Inputfile(filename)){
}

IStatus * StatusFactory::createStatus() {
    IStatus * status;
    if (myStatusType == SimpleStatusType) {
        status = new SimpleStatus(period, chargeRate, maxPower);
    }
    else {
        cerr << "Unknown status type:" << myStatusType << endl;
        return NULL;
    }
    string str;
    if (inputfile->readNextLine(str)) {
        status->parseStatusString(str);
    }
    else {
        delete status;
        return NULL;
    }
    return status;
}

StatusFactory::~StatusFactory() {
}

