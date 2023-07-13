import socket
import time

filename = "/home/vlad/Documents/album_test.png"
lot_id = 42069

TCP_IP = '127.0.0.1'
TCP_PORT = 52004
TCP_PORT_PSD = 52002

with open(filename, "rb") as f:
    data = f.read()
data_len = len(data)
message = "newfile"+str(filename.split('/')[-1]+"|"+str(lot_id)+"|"+str(data_len))
print(message)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
s.send(bytes(message, "UTF-8"))
time.sleep(1)
s.send(data)

s.close()
time.sleep(1)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT_PSD))
message = "album"+str(lot_id)
print(message)
s.send(bytes(message, "UTF-8"))
s.close()