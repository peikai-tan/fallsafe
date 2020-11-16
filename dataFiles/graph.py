import os
import csv
from matplotlib import pyplot as plt

folders = ["running", "walking", "stationary", "jumping", "falling"]

option = -1

os.chdir(os.getcwd() + "/dataFiles/")

# for folder in folders:
files = os.listdir(folders[option])

for f in files:
    print(f)
    f = csv.reader(open(folders[option] + "/" + f, "r"))
    next(f)

    dataset = [[float(dat) for dat in row] for row in f]

    plt.plot(range(len(dataset)), dataset)
    plt.show()
