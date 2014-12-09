/*
 * CMStatusWriter.cpp
 *  Write out the service records and node status, such as the residue power.
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include "CMStatusWriter.h"

CMStatusWriter::CMStatusWriter(const string & filename) {
    outputfile = new Outputfile(filename);
}

bool CMStatusWriter::writeStatus(string & statusstr) {
    outputfile->writeLine(statusstr);
    return true;
}

bool CMStatusWriter::writeStatus(IStatus * status[], int num) {
    string line;
    char tmp[20];
    // Get the average.
    double sum;
    int j = 0;
    cout << "wirteStatus:AAA" << endl;
    for (int k = 0; k < 3; k ++) {
        sum = 0;
        for (int i = 0; i < EACH_CM_GROUP; i ++) {
            sum += status[i + j]->getPower();
        }
        j += EACH_CM_GROUP;
        sum /= EACH_CM_GROUP;
        sprintf(tmp, "%.2lf ", sum);
        line = line + tmp;
    }
    cout << "wirteStatus:BBB" << endl;
    fflush(stdout);
    return writeStatus(line);
}

CMStatusWriter::~CMStatusWriter() {
    delete outputfile;
}

