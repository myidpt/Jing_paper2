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

package jingqin.header;

//
// TODO auto-generated module
//
simple CH
{
    parameters:
        @display("i=device/pc2");
        bool sendInitialMessage = true;
        int numCM;
        int numSensor;
        int printStatusStep;
        string CH_task_input_filename;
        string CH_task_output_filename;
        string CM_status_input_filename;
        string CM_status_output_filename;
        string CH_rt_task_stats;
        string algorithm;
        double period;
        double chargeRate;
        double maxPower;
    gates:
        inout gate[];
}
