/*
 * Outputfile.h
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#ifndef OUTPUTFILE_H_
#define OUTPUTFILE_H_

#include <iostream>
#include <fstream>

using namespace std;

class Outputfile {
protected:
    ofstream * stream;
public:
    Outputfile(const string & filename);
    bool writeLine(const string & line);
    virtual ~Outputfile();
};

#endif /* OUTPUTFILE_H_ */
