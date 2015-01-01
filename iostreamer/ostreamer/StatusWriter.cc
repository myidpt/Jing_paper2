/*
 * StatusWriter.cpp
 *  Write out the service records and node status, such as the residue power.
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include "StatusWriter.h"
#define NOW SIMTIME_DBL(simTime())

StatusWriter::StatusWriter(const string & filename) {
    outputfile = new Outputfile(filename);
}

void StatusWriter::writeStatus(const string & statusstr) {
    outputfile->writeLine(statusstr);
}

void StatusWriter::writeStatus(IStatus * status) {
    char line[100];
    // Get the average.
    sprintf(line, "%.2lf %.2lf", NOW, status->getPower());
    string strline = line;
    writeStatus(strline);
}

StatusWriter::~StatusWriter() {
    delete outputfile;
}

