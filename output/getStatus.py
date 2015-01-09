#!/usr/bin/env python

import sys

if (len(sys.argv) != 4):
  print sys.argv[0], "<input file> <output file> <step>"
  sys.exit()

finname = sys.argv[1]
foutname = sys.argv[2]
step = float(sys.argv[3])

timestep = 0.0
lasttime = 0.0
lastpower = 1000.00

with open(finname, "r") as fin, open(foutname, "w") as fout:
  while True:
    line = fin.readline()
    if line == '':
      break
    elem = line.strip().split(" ")
    time = float(elem[0])
    power = float(elem[1])
    if timestep == time:
      fout.write("%(t).0f %(p).2f\n" %{"t":timestep, "p":power})
      timestep = timestep + step
    elif time > timestep:
      timespan = time - lasttime
      powerspan = power - lastpower
      while True:
        powerstep = lastpower + powerspan / timespan * (timestep - lasttime)
        fout.write("%(t).0f %(p).2f\n"%{"t":timestep, "p":powerstep})
        timestep = timestep + step
        if timestep > time:
          break
    lasttime = time
    lastpower = power
