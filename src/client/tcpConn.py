# This Python file uses the following encoding: utf-8

import socket, threading
from PySide2.QtCore import qDebug as qPrint
from time import sleep

BUFFER_SIZE = 1024

class Connection():
    def __init__(self, address, port, writeback):
        self.address = address
        self.port = port
        self.writeback = writeback
        self.online = False
        self.readThread = None
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_thread = threading.Thread(target=self.start_socket, args=())
        self.socket_thread.start()

    def start_socket(self):
        print("Attempting connection")
        self.socket.settimeout(5)
        count = 3
        connected = False
        while not connected:
            try:
                self.socket.connect((self.address, self.port))
                connected = True
            except Exception as e:
                self.writeback.addText("Error establishing connection: {}".format(e))
                if count > 0:
                    self.writeback.addText("Retrying ({})...".format(count))
                    count = count - 1
                else:
                    self.writeback.addText("Too many retries.")
                    return
        self.online = True

        self.socket.settimeout(None)
        self.writeback.addText("Connection Established")
        while self.online:
            try:
                buffer = self.socket.recv(BUFFER_SIZE)
                #qPrint("Reading {}:{}".format(len(buffer), id(buffer)))
                if len(buffer) > 0:
                    self.writeback.addText(buffer.decode("utf-8"))
                else:
                    raise Exception("Server returned empty packet (likely shutdown)")
            except Exception as e:
                self.writeback.addText("Error while reading from socket: {}".format(e))
                qPrint("Closing")
                self.online = False

    def socket_write(self, data):
        try:
            self.socket.send(data.encode('utf-8'))
        except Exception as e:
            self.writeback.addText('<p style="color:#ff0000";>Command "{}" could not be sent: {}.</p>'.format(data, e))

    def close_external(self):
        self.online = False
        try:
            self.socket.close()
        except Exception as e:
            qPrint("Exception when closing socket: {}".format(e))
        try:
            if self.readThread != None:
                self.readThread.join()
        except Exception as e:
            qPrint("Exception when joining read thread: {}".format(e))



# if__name__ == "__main__":
#     pass
