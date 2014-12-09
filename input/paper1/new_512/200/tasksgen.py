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
delay = 200

def exp(ave):
  return - ave * logn(e, 1-random.random())

id = 0
max = 3000
ave1 = 80
ave2 = 40
time1 = exp(ave1)
time2 = exp(ave2)
while id < max:
  if time1 <= time2:
    f.write('%(id)d %(time)lf 32 0 1 1 %(compute)d %(delay)d\n' 
        %{"id":id,"time":time1,"compute":compute,"delay":delay})
    time1 = time1 + exp(ave1)
  else:
    f.write('%(id)d %(time)lf 32 1 1 1 %(compute)d %(delay)d\n' 
        %{"id":id,"time":time2,"compute":compute,"delay":delay})
    time2 = time2 + exp(ave2)
  id = id + 1

f.close()

