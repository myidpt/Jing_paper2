/*
 * Inputfile.cpp
 *
 *  Created on: Apr 9, 2013
 *      Author: yonggang
 */

#include "Inputfile.h"

Inputfile::Inputfile(const string & filename) {
    stream = new ifstream();
    stream->open(filename.c_str());
    cout << "Open task file for input: " << filename << endl;
    if (stream->fail())
    {
        cerr << "Cannot open file " << filename << endl;
        stream = NULL;
    }
}

bool Inputfile::readNextLine(string & line) {
    if (stream == NULL) {
        cout << "Stream is NULL." << endl;
        return false;
    }

    // Read the next line.
    while ( stream != NULL && stream->good() ) { // Not reached the eof.
        getline(*stream, line);
        if (line[0] != '\n' && line[0] != '\0' && line[0] != '#') {
            return true;
        }
    }

    // Check if it needs to be closed.
    if (stream != NULL && stream->eof()){
        stream->close();
        delete stream; // Dispose the pointer.
        stream = NULL;
    }
    return false;
}

Inputfile::~Inputfile() {
    if(stream != NULL) {
        stream->close();
        delete stream;
    }
}

