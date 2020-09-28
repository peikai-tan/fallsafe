from sense_hat import SenseHat
import math
import time
import csv

run = False
sense = SenseHat()
timestr = time.strftime("%d-%m-%Y-%H:%M:%S")

while not run:
    for event in sense.stick.get_events():
        if event.action == "pressed" and event.direction == "middle":
            run = True
            sense.set_pixels([[100, 0, 0] for x in range(64)])

f = open(timestr+".csv", "w", newline="")
dataCSV = csv.writer(f)

labels = ["x", "y", "z", "mag"]
dataCSV.writerow(labels)

while run:
    data = sense.get_accelerometer_raw()
    x = data["x"]
    y = data["y"]
    z = data["z"]

    mag = (math.sqrt(x*x + y * y + z * z))
    dataCSV.writerow([x, y, z, mag])

    for event in sense.stick.get_events():
        if event.action == "pressed" and event.direction == "middle":
            run = False
            sense.set_pixels([[0, 0, 0] for x in range(64)])
f.close()
