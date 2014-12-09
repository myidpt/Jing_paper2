/*
 * Inputfile.h
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#ifndef INPUTFILE_H_
#define INPUTFILE_H_

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Inputfile {
protected:
    ifstream * stream;
public:
    Inputfile(const string & filename);
    bool readNextLine(string & line);
    virtual ~Inputfile();
};

#endif /* INPUTFILE_H_ */
