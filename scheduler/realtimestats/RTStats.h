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

#ifndef RTSTATS_H_
#define RTSTATS_H_

#include <set>
#include <map>
#include "General.h"

using namespace std;

class RTStats {
protected:
    int startNum;
    map<double, int> startNumExps; // expiration time, num
    double period;
public:
    class StatMark {
    public:
        typedef multiset<StatMark, bool (*)(const StatMark&, const StatMark&)> StatSet;

        static StatSet* allStats;

        static bool compareNode(const RTStats::StatMark& lhs, const RTStats::StatMark& rhs)
        {
           return lhs < rhs;
        }

        double time;
        int change;
        int type;
        double expiration;
        // -1 means its Realtime.
        // Otherwise, it means the time non-RT task comes/goes.
        StatMark(double ti, int n, int ty)
        : time(ti), change(n), type(ty), expiration(-1){}// RT
        StatMark(double ti, int n, int ty, double exp)
        : time(ti), change(n), type(ty), expiration(exp){} // NRT
        void print() const {
            cout << "Time: " << time << ", change: " << change << ", type: " << type
                 << ", expiration: " << expiration << endl;
        }
        bool operator<(const StatMark& other) const {
            if (time == other.time) {
                return change < other.change;
            }
            return time < other.time;
        }
    };

    RTStats(double period);
    void readTaskStats(const char  * filename);
    int getMostTaskInInterval(double, double);
    // This function also updates the list of nrt tasks.
    // Remove old ones and insert the new one.
    void addTask(double now, double time, int num, double duration, int type);
    void print();
    void test();
    virtual ~RTStats();
};

#endif /* RTSTATS_H_ */
