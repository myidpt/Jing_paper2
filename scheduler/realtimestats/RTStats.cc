//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <stdio.h>
#include <string>
#include <map>
#include <assert.h>
#include "Inputfile.h"
#include "RTStats.h"

using namespace std;

RTStats::StatMark::StatSet* RTStats::StatMark::allStats = NULL;

RTStats::RTStats(double p) {
    startNum = 0;
    period = p;
    StatMark::allStats = new StatMark::StatSet(StatMark::StatMark::compareNode);
}

void RTStats::readTaskStats(const char  * filename) {
    Inputfile inputfile(filename);
    string line;
    double time;
    int tasks;
    double duration;
    int type;
    while(inputfile.readNextLine(line)) {
        sscanf(line.c_str(), "%lf %d %lf %d", &time, &tasks, &duration, &type);
        StatMark rts1(time, tasks, type);
        StatMark::allStats->insert(rts1);
        StatMark rts2(time + duration, -tasks, type);
        StatMark::allStats->insert(rts2);
    }
}

int RTStats::getMostTaskInInterval(double time, double duration) {
    double start = time - (int)(time / period) * period;
    double end = time + duration - (int)((time + duration) / period) * period;
    // Find the most.
    int most = -1;
    int activetasks = startNum;
    RTStats::StatMark::StatSet::iterator it;
    if (start < end) {
        for(it = StatMark::allStats->begin(); it != StatMark::allStats->end(); ++it) {
            StatMark sm = *it;
            if (sm.time >= end) {
                break;
            }
            if (sm.change > 0) {
                activetasks += sm.change;
                if (start <= sm.time && sm.time < end) {
                    if (activetasks > most) {
                        most = activetasks;
                    }
                }
            }
            else {
                if (start < sm.time && sm.time < end) {
                    if (activetasks > most) {
                        most = activetasks;
                    }
                }
                activetasks += sm.change;
            }
        }
        if (most == -1) {
            most = activetasks;
        }
        return most;
    }
    else {
        most = activetasks;
        for(it = StatMark::allStats->begin(); it != StatMark::allStats->end(); ++it) {
            StatMark sm = *it;
            if (sm.change > 0) {
                activetasks += sm.change;
                if (sm.time < end) {
                    if (activetasks > most) {
                        most = activetasks;
                    }
                }
                if (sm.time >= start) {
                    if (activetasks > most) {
                        most = activetasks;
                    }
                }
            }
            else {
                if (sm.time < end) {
                    if (activetasks > most) {
                        most = activetasks;
                    }
                }
                if (sm.time >= start) {
                    if (activetasks > most) {
                        most = activetasks;
                    }
                }
                activetasks += sm.change;
            }
        }
        if (activetasks > most) { // Prevent no update point after start.
            most = activetasks;
        }
        return most;
    }
}

void RTStats::addTask(double now, double time, int num, double duration, int type) {
    double start = time - (int)(time / period) * period;
    double end = time + duration - (int)((time + duration) / period) * period;
    if (start > end) {
        map<double, int>::iterator sit = startNumExps.find(time + duration);
        if (sit == startNumExps.end()) {
            startNumExps.insert(pair<double, int>(time + duration, num));
        }
        else {
            sit->second += num;
        }
        startNum += num;
    }
    StatMark rts1(start, num, type, time + duration);
    StatMark::allStats->insert(rts1);
    StatMark rts2(end, -num, type, time + duration);
    StatMark::allStats->insert(rts2);

    // Clean old ones.
    bool no_more = false;
    while (no_more == false) {
        no_more = true;
        for(StatMark::StatSet::iterator it = StatMark::allStats->begin();
                it != StatMark::allStats->end(); ++ it) {
            StatMark sm = *it;
            if (sm.expiration > 0 && sm.expiration <= now) {
                no_more = false;
                //cout << "Erase: " << sm.time << ", change=" << sm.change << endl;
                StatMark::allStats->erase(it);
            }
        }
    }

    if (startNum > 0) {
        int startNumOff = 0;
        for(map<double, int>::iterator sit = startNumExps.begin();
                sit != startNumExps.end(); ++sit) {
            if (sit->first <= now) {
                startNumOff += sit->second;
                startNumExps.erase(sit);
            }
        }
        startNum -= startNumOff;
    }
}

void RTStats::print() { // Print
    cout << "startNum = " << startNum << endl;
    for (RTStats::StatMark::StatSet::iterator it = StatMark::allStats->begin();
        it != StatMark::allStats->end(); ++ it) {
        it->print();
    }
}

void RTStats::test() {
    for(StatMark::StatSet::iterator it = StatMark::allStats->begin();
            it != StatMark::allStats->end(); ++it) {
        StatMark::allStats->erase(it);
    }
    period = 5000;
    addTask(30, 30.0, 3, 70.0, 1);
    assert(getMostTaskInInterval(0,30) == 0);
    assert(getMostTaskInInterval(30,100) == 3);
    addTask(40, 40.0, 6, 100.0, 1);
    assert(getMostTaskInInterval(30,10) == 3);
    assert(getMostTaskInInterval(35,10) == 9);
    addTask(40, 40.0, 8, 10.0, 1);
    assert(getMostTaskInInterval(40,10) == 17);
    assert(getMostTaskInInterval(30,1000) == 17);
    assert(getMostTaskInInterval(300,1000) == 0);
    addTask(4998, 4998, 10, 3, 1);
    assert(startNum == 10);
    assert(getMostTaskInInterval(5000,10) == 10); // [0,1]:10
    assert(getMostTaskInInterval(4999,10) == 10); // [0,1]:10
    addTask(4999, 4999, 10, 3, 1);
    assert(startNum == 20);
    assert(getMostTaskInInterval(4998,10) == 20); // [0,1]:10, [0,2]:10
    assert(getMostTaskInInterval(4999,10) == 20); // [0,1]:10, [0,2]:10
    assert(getMostTaskInInterval(1,2) == 10);
    addTask(5001, 5001, 6, 10, 1);
    assert(startNum == 10); // [4998, 5001] is removed.
    assert(getMostTaskInInterval(0,10) == 16);
    assert(getMostTaskInInterval(30,60) == 0);
}

RTStats::~RTStats() {
}

