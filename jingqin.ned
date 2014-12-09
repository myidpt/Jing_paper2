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

package jingqin;

import jingqin.header.CH;
import jingqin.member.CM;

network jingqin
{
    parameters:
        int numCM;
    types:
        channel Channel extends ned.DatarateChannel
        {
            delay = 0 ms;
            datarate = 0 Mbps;
        }
    submodules:
        cm[numCM]: CM {
            @display("p=227,214");
        }
        ch: CH {
            @display("p=109,91");
            gates:
                gate[numCM];
        }
    connections:
        for i=0..numCM-1 {
            cm[i].gate <--> Channel <--> ch.gate[i];
        }
}
