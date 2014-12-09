#!/usr/bin/env python

import sys
import random

f = open('status', 'w')
id = 0
compcap = 1
energy = 3000

random.seed()

for i in range (0, 500):
  #compcap = random.random() * 0.4 + 0.6
  compcap = 1
  f.write('%(id)d;%(compcap)lf;%(power)ld;0:1,1:1\n' %{"id":id,"compcap":compcap,"power":energy})
  id = id + 1

f.close()

