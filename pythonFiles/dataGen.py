from sense_hat import SenseHat
import math
import time
import csv
import atexit


def toMessWithChalk(angle):
    coors = list()

    def drawLine(x1, x2, y1, y2):
        m = (y2 - y1) / (x2 - x1)
        return [[x, round(m*x)] for x in range(-4, 4)]

    def pixelise(coors):
        results = []
        blank = [0, 0, 0]
        red = [100, 0, 0]
        for layerOne in coors:
            grid = [blank for p in range(64)]
            for layerTwo in layerOne:
                grid[32 + layerTwo[0] + (layerTwo[1] * 8)] = red
            results.append(grid)

        return results

    for a in range(0, angle, 5):
        r = a * math.pi/180
        x = math.cos(r)
        y = math.sin(r)

        coors.append(drawLine(x, -x, y, -y) + drawLine(x, -x, -y, y))

    return pixelise(coors)


crosses = toMessWithChalk(30)
xc = 0


red = [100, 0, 0]
green = [0, 100, 0]
blank = [0, 0, 0]

acts = ["walking", "running", "jumping",
        "stationary", "e"]  # Expected activities

sense = SenseHat()
atexit.register(sense.clear)

while True:
    run = False
    c = 0
    sense.show_letter(acts[c][0], green)

    while not run:
        for event in sense.stick.get_events():
            if event.action == "pressed":
                d = event.direction
                if d == "middle":
                    run = True if acts[c] != "e" else exit()
                    continue
                c = (
                    c + 1) % len(acts) if d in ["right", "up"] else (c-1) % len(acts)
                sense.show_letter(acts[c][0], green)

    # Creating csv file for logging of data
    file = "./dataFiles/" + acts[c] + "/" + acts[c] + \
        "-" + time.strftime("%d-%m-%Y-%H:%M:%S") + ".csv"
    f = open(file, "w", newline="")
    dataCSV = csv.writer(f)

    labels = ["x", "y", "z", "mag"]
    dataCSV.writerow(labels)

    while run:
        sense.set_pixels(crosses[xc])
        xc = (xc + 1) % len(crosses)
        data = sense.get_accelerometer_raw()
        x = data["x"]
        y = data["y"]
        z = data["z"]

        mag = math.sqrt(x * x + y * y + z * z)
        dataCSV.writerow([x, y, z, mag])

        for event in sense.stick.get_events():
            run = False if event.action == "pressed" and event.direction == "middle" else True

    sense.set_pixels([[0, 0, 0] for x in range(64)])
    f.close()
