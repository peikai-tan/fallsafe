import os
import csv
import numpy as np
from matplotlib import pyplot as plt

os.chdir(os.getcwd() + "/dataFiles/")
print(os.getcwd())
files = os.listdir("walking")


for f in files:
    if f != "long":
        print(f)
        f = csv.reader(open("walking/" + f, "r"))

        mags = [mag[-1] for mag in f]
        mags = mags[1:]

        mags = np.array([float(mag) for mag in mags])
        mags = mags - 1
        mags = mags[:1000]

        mags = mags[int(len(mags) * 0.1):int(len(mags) * 0.9)]

        plt.plot(range(len(mags)), mags)
        plt.show()
