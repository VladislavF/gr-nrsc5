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
info_content = "|"+str(filename.split('/')[-1]+"|"+str(lot_id)+"|"+str(data_len))
message = "newfile"+str(len(info_content)+len("newfile"))+info_content
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
message = "lot"+str(lot_id)+"\n"
print(message)
s.send(bytes(message, "UTF-8"))
s.close()