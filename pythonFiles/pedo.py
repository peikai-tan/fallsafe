import csv
from matplotlib import pyplot as plt

f = csv.reader(open("data.csv", "r"))

mags = [mag[-1] for mag in f]
mags = mags[1:]

mags = [float(mag) for mag in mags]

print(mags)

plt.plot(range(len(mags)), mags)
plt.show()
