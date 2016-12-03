#!/usr/bin/python27

import numpy
import matplotlib
import numpy as np
import matplotlib.pyplot as plt

data = np.genfromtxt('data.csv', delimiter=',', names=['x', 'umap', 'ht', 'mic', 'goo'])
plt.plot(data['x'], data['umap'], '*', color='r', label='std::unordered_map')
plt.plot(data['x'], data['ht'], '*', color='b', label='ht')
plt.plot(data['x'], data['mic'], '*', color='g', label='boost::mic')
plt.plot(data['x'], data['goo'], '*', color='c', label='google::dense_hash_map')



