#!/usr/bin/env python

import sys
import random
from numpy.lib.scimath import logn
from math import e

random.seed()

f = open('tasks', 'w')
f.write('# SensorID: rate, sensorID: rate, ...\n')
f.write('0:1,1:2\n')
f.write('# Id time num_sub sensor input output compute delay\n')

id = 0
time = 0
compute = 3200
delay = 1600

def exp(ave):
  return - ave * logn(e, 1-random.random())

id = 0
max = 5000
ave1 = 20
time1 = exp(ave1)
while id < max:
  f.write('%(id)d %(time)lf 32 1 1 1 %(compute)d %(delay)d\n' 
      %{"id":id,"time":time1,"compute":compute,"delay":delay})
  time1 = time1 + exp(ave1)
  id = id + 1

f.close()

