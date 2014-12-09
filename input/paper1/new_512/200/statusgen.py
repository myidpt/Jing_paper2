import sys
import random

f = open('status', 'w')
id = 0
compcap = 1
energy = 5000

random.seed()

for i in range (0, 512):
  compcap = random.random() * 0.4 + 0.6
  f.write('%(id)d;%(compcap)lf;%(power)lf;0:1\n' %{"id":id,"compcap":compcap,"power":energy})
  id = id + 1

for i in range (0, 512):
  compcap = random.random() * 0.4 + 0.6
  f.write('%(id)d;%(compcap)lf;%(power)lf;1:1\n' %{"id":id,"compcap":compcap,"power":energy})
  id = id + 1

for i in range (0, 512):
  compcap = random.random() * 0.4 + 0.6
  f.write('%(id)d;%(compcap)lf;%(power)lf;0:1,1:1\n' %{"id":id,"compcap":compcap,"power":energy})
  id = id + 1

f.close()

