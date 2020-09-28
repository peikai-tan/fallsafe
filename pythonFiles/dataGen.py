from sense_hat import SenseHat
import math
import time
import csv


sense = SenseHat()

f = open("data.csv", "w", newline="")
dataCSV = csv.writer(f)

labels = ["x", "y", "z", "mag"]
dataCSV.writerow(labels)

while True:
    data = sense.get_accelerometer_raw()
    x = data["x"]
    y = data["y"]
    z = data["z"]

    mag = (math.sqrt(x*x + y * y + z * z))
    dataCSV.writerow([x, y, z, mag])

f.close()
