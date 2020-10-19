import socket
import select
import sys
import re

import threading

from matplotlib import pyplot as plt
from matplotlib import animation

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


fig, axs = plt.subplots(2)

xs = []
ys = []


class Listener():
    def __init__(self, my_username, IP, PORT):
        self.acc = []
        self.ori = []

        self.HEADER_LENGTH = 10

        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.server_socket.connect((IP, PORT))
        self.server_socket.setblocking(False)

        username = my_username.encode('utf-8')
        username_header = f"{len(username):<{self.HEADER_LENGTH}}".encode(
            'utf-8')
        self.server_socket.send(username_header + username)

    def getData(self):
        return self.acc, self.ori

    def setData(self):
        try:
            message = self.server_socket.recv(60).decode("utf-8")

            x = float(message[0:10])
            y = float(message[10:20])
            z = float(message[20:30])

            self.acc.append([x, y, z])

            p = float(message[30:40]) * 180 / math.pi
            r = float(message[40:50]) * 180 / math.pi
            y = float(message[50:60]) * 180 / math.pi

            self.ori.append([p, r, y])

        except:
            pass


def updateListener(listener):
    while True:
        listener.setData()


def update(i, xs, ys, listener):
    try:
        acc, ori = listener.getData()

        ys = acc[-50:]
        xs = [x for x in range(len(ys))]

        axs[0].clear()
        axs[0].plot(xs, ys)
        axs[0].set_ylim([-3, 3])
        axs[0].legend(('X', 'Y', 'Z'))
        axs[0].set_title("Accelerator (Deg)")

        ys = ori[-50:]
        xs = [x for x in range(len(ys))]

        axs[1].clear()
        axs[1].plot(xs, ys)
        axs[1].set_ylim([-180, 180])
        axs[1].legend(('Pitch', 'Roll', 'Yaw'))
        axs[1].set_title("Orientation (Deg)")

    except:
        pass


listener = Listener(my_username, IP, PORT)

threading.Thread(group=None, target=updateListener, args=(listener,)).start()

ani = animation.FuncAnimation(
    fig, update, fargs=(xs, ys, listener), interval=100)
plt.show()
