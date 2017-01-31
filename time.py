#!/usr/bin/python
 
import sys
import matplotlib.pyplot as plt

for arg in sys.argv[1:]:
    samples = open(arg).readlines()[0].split(',')
    for x in samples:
        x = float(x)

    plt.plot(samples)

plt.show()

