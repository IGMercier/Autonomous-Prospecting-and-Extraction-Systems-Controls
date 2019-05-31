# This Python file uses the following encoding: utf-8

import socket, threading
from PySide2.QtCore import qDebug as qPrint
from time import sleep

BUFFER_SIZE = 1024

messages = 0

class Connection():
    def __init__(self, address, port, writeSignal, onlineState):
        self.address = address
        self.port = port
        self.writeback = writeSignal
        self.online = onlineState
        self.active = False
        self.connected = False
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_thread = threading.Thread(target=self.start_socket, args=())
        self.socket_thread.start()

    def reconnect(self):
        # ASSERT: active == False and connected == False
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_thread = threading.Thread(target=self.start_socket, args=())
        self.socket_thread.start()

    def start_socket(self):
        self.socket.settimeout(5)
        count = 3
        active, connected = True, False
        self.online.emit(active, connected)
        while True:
            try:
                self.socket.connect((self.address, self.port))
                break
            except Exception as e:
                self.writeback.emit("Error establishing connection: {}".format(e))
                if count > 0:
                    self.writeback.emit("Retrying ({})...".format(count))
                    count = count - 1
                else:
                    self.writeback.emit("Too many retries.")
                    active, connected = False, False
                    self.online.emit(active, connected)
                    return
        active, connected = True, True
        self.online.emit(active, connected)

        self.socket.settimeout(None)
        self.writeback.emit("Connection Established")
        while True:
            try:
                buffer = self.socket.recv(BUFFER_SIZE)
                if len(buffer) > 0:
                    self.writeback.emit(buffer.decode("utf-8"))
                else:
                    raise Exception("Server returned empty packet (likely shutdown)")
            except Exception as e:
                self.writeback.emit("Error while reading from socket: {}({})".format(type(e).__name__, e))
                active, connected = False, False
                self.online.emit(active, connected)
                break

    def socket_write(self, data):
        try:
            self.socket.send(data.encode('utf-8'))
        except Exception as e:
            self.writeback.emit('<p style="color:#ff0000";>Command "{}" could not be sent: {}({}).</p>'.format(data, type(e).__name__, e))

    def close(self):
        try:
            self.socket.settimeout(2)
            self.socket.close()
        except Exception as e:
            qPrint("Exception when closing socket: {}({})".format(type(e).__name__, e))
        try:
            if self.socket_thread != None:
                self.socket_thread.join()
        except Exception as e:
            qPrint("Exception when joining read thread: {}({})".format(type(e).__name__, e))
        active, connected = False, False
        self.online.emit(active, connected)



# if__name__ == "__main__":
#     pass
