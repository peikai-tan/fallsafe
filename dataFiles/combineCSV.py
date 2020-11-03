import os
import csv
import numpy as np

folders = ["running", "walking", "stationary", "jumping", "falling"]

option = -1

os.chdir(os.getcwd() + "/dataFiles/")

files = os.listdir(folders[option])

dataset = []
for f in files:
    print(f)
    f = csv.reader(open(folders[option] + "/" + f, "r"))
    next(f)

    for row in f:
        dataset.append([float(dat) for dat in row])


np.savetxt(folders[option] + "/combined-" + folders[option] + ".csv", dataset, delimiter=",", fmt="%02.10f")