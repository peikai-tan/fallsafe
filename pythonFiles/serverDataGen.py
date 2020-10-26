
import socket
import select

from sense_hat import SenseHat

sense = SenseHat()


HEADER_LENGTH = 10

IP = "0.0.0.0"
PORT = 1234

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind((IP, PORT))
server_socket.listen()
sockets_list = [server_socket]

print(f'Listening..')


def receive_message(client_socket):
    try:
        message_header = client_socket.recv(HEADER_LENGTH)
        if not len(message_header):
            return False
        message_length = int(message_header.decode('utf-8').strip())
        return {'header': message_header, 'data': client_socket.recv(message_length)}

    except:
        return False


def send(client):
    print('Sending..')
    while True:
        try:
            acc = sense.get_accelerometer_raw()
            x = str(acc["x"])[:10]
            y = str(acc["y"])[:10]
            z = str(acc["z"])[:10]

            ori = sense.get_orientation_radians()
            pitch = str(ori["pitch"])[:10]
            roll = str(ori["roll"])[:10]
            yaw = str(ori["yaw"])[:10]

            message = x+y+z+pitch+roll+yaw
            client.send(message.encode("utf-8"))
        except:
            break


while True:
    read_sockets, _, exception_sockets = select.select(
        sockets_list, [], sockets_list)

    for notified_socket in read_sockets:
        if notified_socket == server_socket:
            client, client_address = server_socket.accept()
            user = receive_message(client)
            if user is False:
                continue
            sockets_list.append(client)
            print('Accepted new connection from {}:{}, username: {}'.format(
                *client_address, user['data'].decode('utf-8')))
            send(client)
            print('{} disconnected.'.format(
                user['data'].decode('utf-8')))
