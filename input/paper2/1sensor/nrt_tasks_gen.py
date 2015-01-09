#!/usr/bin/env python

import sys
import random
from numpy.lib.scimath import logn
from math import e

random.seed()

f = open('nrt_tasks', 'w')
f.write('# SensorID: rate, sensorID: rate, ...\n')
f.write('0:1,1:2\n')
f.write('# Id time num_sub sensor input output compute QoS_latency\n')

id = 0
time = 0
compute = 3200
num = 32
delay = 1600

ave = 100

def gen_time():
  return time - ave * logn(e, 1-random.random())

id = 0
max = 30000
time = gen_time()
sensor = 0
while id < max:
  f.write('%(id)d %(time).2lf %(num)d %(sensor)d 1 1 %(compute)d %(delay)d\n'
      %{"id":id,"time":time,"num":num,"sensor":sensor,"compute":compute,"delay":delay})
  time = gen_time()
  id = id + 1

f.close()
