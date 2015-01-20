#!/usr/bin/env python

import sys
import random
from numpy.lib.scimath import logn
from math import e

random.seed()

f = open('rt_tasks', 'w')
f.write('# arrival_time_in_each_period rt_task_number total_cost resource_type\n')

id = 0
time = 0
cost = 25600
num = 64
step = 675

def gen_time():
  return time + step

max_time = 5400 - cost / num
time = gen_time()
sensor = 0
while time < max_time:
  f.write('%(time).2lf %(num)d %(cost)d %(sensor)d\n'
      %{"time":time,"num":num,"cost":cost,"sensor":sensor})
  time = gen_time()

f.close()
