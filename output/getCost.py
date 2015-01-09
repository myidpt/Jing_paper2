#!/usr/bin/env python

import sys

if (len(sys.argv) != 8):
  print sys.argv[0], "<initial> <period> <inputf> <outputf> <step> <chargerate> <nodes>"
  sys.exit()

apower = float(sys.argv[1])
period = float(sys.argv[2])
finname = sys.argv[3]
foutname = sys.argv[4]
step = float(sys.argv[5])
chargerate = float(sys.argv[6])
nodes = int(sys.argv[7])
apower = apower * nodes
lasttpower = 0

with open(finname, "r") as fin, open(foutname, "w") as fout:
  while True:
    line = fin.readline()
    if line == '':
      break
    elem = line.strip().split(" ")
    time = float(elem[0])
    rpower = float(elem[1])
    offset = time - (int)(time / period) * period
    if offset <= period/2 and offset != 0:
      apower = apower + step * chargerate * nodes
    tpower = apower - rpower
    fout.write("%(t).0f %(p).2f\n" %{"t":time, "p":(tpower-lasttpower)})

    lasttpower = tpower
