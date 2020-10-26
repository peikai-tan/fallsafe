import socket
import select
import sys
import re

import numpy as np
from tensorflow import keras
import os

import math

regex = '''^(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.( 
            25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.( 
            25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.( 
            25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)$'''

if len(sys.argv) != 2:
    raise TypeError("Not enough arguments provided")
elif not re.search(regex, sys.argv[1]):
    raise ValueError("Make sure you keyed in a valid IP address.")
print("Connecting..")

IP = sys.argv[1]
PORT = 1234
my_username = socket.gethostname()

model = keras.models.load_model("pythonFiles/savedModel")


class Listener():
    def __init__(self, my_username, IP, PORT, length):
        self.x = [0] * length
        self.y = [0] * length
        self.z = [0] * length

        self.length = length

        self.HEADER_LENGTH = 10

        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.server_socket.connect((IP, PORT))
        self.server_socket.setblocking(False)

        username = my_username.encode('utf-8')
        username_header = f"{len(username):<{self.HEADER_LENGTH}}".encode(
            'utf-8')
        self.server_socket.send(username_header + username)

    def getData(self):
        self.x = self.x[-self.length:]
        self.y = self.y[-self.length:]
        self.z = self.z[-self.length:]
        return np.array([self.x + self.y + self.z])

    def setData(self):
        try:
            message = self.server_socket.recv(60).decode("utf-8")

            self.x.append(float(message[0:10]))
            self.y.append(float(message[10:20]))
            self.z.append(float(message[20:30]))

        except:
            pass


length = 60
listener = Listener(my_username, IP, PORT, length)

choices = ["walk", "stationary"]
while True:
    listener.setData()
    data = listener.getData()
    prediction = model.predict(data)
    result = np.argmax(prediction)

    os.system("clear")
    print(choices[result])
    print("Confidence: {0}".format(round(prediction[0][result]*100, 2)))
