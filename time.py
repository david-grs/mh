#!/usr/bin/python
 
import sys
import matplotlib.pyplot as plt

samples = open(sys.argv[1]).readlines()[0].split(',')
samples.pop()
for x in samples:
    x = float(x)

plt.plot(samples)
plt.show()

