#!/usr/bin/env python

import sys

if (len(sys.argv) != 3):
  print sys.argv[0], "<CM num> <sensor num>"
  sys.exit()

fname = "status"
cnum = int(sys.argv[1])
snum = int(sys.argv[2])

sensors = ""
for i in range(0, snum):
  if i is not 0:
    sensors = sensors + ","
  sensors = sensors + str(i) + ":1"

with open(fname, 'w') as f:
  for id in range(0, cnum):
    f.write("%(id)d;1.00;1000;%(s)s\n" %{"id":id,"s":sensors})
