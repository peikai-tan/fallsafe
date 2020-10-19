import os
import csv
import numpy as np
from matplotlib import pyplot as plt

folders = ["running", "walking", "stationary"]

option = 0

os.chdir(os.getcwd() + "/dataFiles/")
print(os.getcwd())
files = os.listdir(folders[option])


for f in files:
    print(f)
    f = csv.reader(open(folders[option] + "/" + f, "r"))
    next(f, None)

    arr = np.array([[float(x) for x in mag] for mag in f])

    plt.plot(range(len(arr)), arr)
    plt.show()
