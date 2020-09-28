from sense_hat import SenseHat
import math
import csv

red = [100, 0, 0]
green = [0, 100, 0]

choices = ["walking", "running", "jumping",
           "stationary"]  # Expected activities
c = 0

sense = SenseHat()

run = False
while not run:
    for event in sense.stick.get_events():
        if event.action == "pressed":
            d = event.direction
            if d == "middle":
                run = True
                sense.set_pixels([red for x in range(64)])
                continue
            c = c + 1 if d == "right" else c - 1
            sense.show_letter(choices[c][0], green)

filename = choices[c] + csv +
f = open("data.csv", "w", newline="")
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
