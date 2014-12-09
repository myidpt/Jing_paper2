/*
 * Outputfile.cpp
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include "Outputfile.h"

Outputfile::Outputfile(const string & filename) {
    stream = new ofstream;
    stream->open(filename.c_str());
    if (stream->fail()) {
        cerr << "Cannot open output file: " << filename << endl;
        stream = NULL;
    }
}

bool Outputfile::writeLine(const string & line) {
    if (stream != NULL && stream->is_open()) {
        *stream << line << endl;
        return true;
    }
    else {
        cout << "Output stream is NULL or not open." << endl;
        return false;
    }
}

Outputfile::~Outputfile() {
    if(stream != NULL) {
        stream->close();
        delete stream;
    }
}

