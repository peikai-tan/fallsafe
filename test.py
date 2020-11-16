from sense_hat import SenseHat
import time

s = SenseHat()

while True:
    print(s.get_accelerometer_raw())
    time.sleep(1)
