#!/usr/bin/env python

import sys

if (len(sys.argv) != 4):
  print sys.argv[0], "<input file1> <input file2> <output file>"
  sys.exit()

finname1 = sys.argv[1]
finname2 = sys.argv[2]
foutname = sys.argv[3]

timestep = 0.0
lasttime = 0.0
lastpower = 1000.00

line1 = 'ABC'
line2 = 'ABC'

with open(finname1, "r") as fin1, open(finname2,"r") as fin2, open(foutname, "w") as fout:
  while True:
    if line1 != '':
      line1 = fin1.readline()
    if line2 != '':
      line2 = fin2.readline()

    if line1 == '' and line2 == '':
      break

    if line1 == '':
      elem = line2.strip().split(" ")
      time = float(elem[0])
      power = float(elem[1])
      fout.write("%(t).0f %(p).2f\n" %{"t":time, "p":power})
      continue
      
    if line2 == '':
      elem = line1.strip().split(" ")
      time = float(elem[0])
      power = float(elem[1])
      fout.write("%(t).0f %(p).2f\n" %{"t":time, "p":power})
      continue

    elem1 = line1.strip().split(" ")
    time1 = float(elem1[0])
    power1 = float(elem1[1])

    elem2 = line2.strip().split(" ")
    time2 = float(elem2[0])
    power2 = float(elem2[1])

    if time1 != time2:
      print time1, time2
      exit(1)

    fout.write("%(t).0f %(p).2f\n" %{"t":time1, "p":power1+power2})
