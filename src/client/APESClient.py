# This Python file uses the following encoding: utf-8
import sys, threading
import tcpConn
from PySide2.QtWidgets import QApplication, QMainWindow, QLabel, QLineEdit, QWidget, QTabWidget, \
                              QVBoxLayout, QToolBar, QFrame, QDialog, QPushButton, QTextEdit, \
                              QHBoxLayout, QCheckBox, QSizePolicy
from PySide2.QtGui  import QTextOption, QTextCursor
from PySide2.QtCore import Signal, Slot, Qt
from PySide2.QtCore import qDebug as qPrint

import datetime # some logging data

class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        QMainWindow.__init__(self)
        self.tabBar = TabBar(self)
        self.setCentralWidget(self.tabBar)
        self.setWindowTitle("APES Client")

    def closeEvent(self, event):
        for i in range(1, len(self.tabBar.tabs)):
            self.tabBar.closeConn(i)
        event.accept()


class TabBar(QTabWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.tabShape = QTabWidget.Rounded
        self.tabs = [StartTab(self)]
        self.addTab(self.tabs[0], "New Connection")
        self.setTabsClosable(True)
        self.tabCloseRequested.connect(self.closeConn)

    @Slot(str, str)
    def newConnection(self, addr, port):
        newConn = ConnTab(self, addr, int(port))
        self.tabs.append(newConn)
        self.addTab(newConn, "{}:{}".format(addr, port))
        self.setCurrentWidget(newConn)

    def closeConn(self, index):
        if index != 0:
            self.tabs[index].close(override=True)
            self.removeTab(index)
            self.tabs.pop(index)

class ConnTab(QDialog):
    writeback = Signal(str, str)
    onlineSignal = Signal(bool, bool)
    def __init__(self, parent, addr, port):
        super().__init__(parent)
        self.parent = parent
        self.connected = False
        mainLayout = QVBoxLayout()
        self.dataBuf = dict()
        self.connLog = ConsoleLog(self.dataBuf, self)
        self.writeback.connect(self.connLog.addText)
        self.writeback.emit("Connecting to {}:{}...".format(addr, port), "")

        submit = QHBoxLayout()
        self.command = commandLine(self)
        self.buttonSend = QPushButton("Send", self)
        self.command.returnPressed.connect(self.buttonSend.click)
        self.buttonSend.clicked.connect(self.sendCommand)
        self.buttonSend.setFixedSize(100, 25)
        submit.addWidget(self.command)
        submit.addWidget(self.buttonSend)
        submit.update()

        recon = QHBoxLayout()
        self.reconButton = QPushButton("Reconnect", self)
        self.reconButton.setEnabled(False)
        self.reconButton.clicked.connect(self.reconnect)
        self.reconAuto = QCheckBox("Auto", self)
        recon.addWidget(self.reconButton)
        recon.addWidget(self.reconAuto)
        
        data = QVBoxLayout()
        sensorWob = QLabel("WOB: N/A", self)
        sensorTmp = QLabel("Temp: N/A", self)
        sensorAmm = QLabel("Ammeter: N/A", self)
        sensorEnc = QLabel("Encoder: N/A", self)
        data.addWidget(sensorWob)
        data.addWidget(sensorTmp)
        data.addWidget(sensorAmm)
        data.addWidget(sensorEnc)
        self.dataBuf["TEMP"] = sensorTmp
        self.dataBuf["CURR"] = sensorAmm
        self.dataBuf["WOB"] = sensorWob
        self.dataBuf["ENCODER"] = sensorEnc

        mainLayout.addWidget(self.connLog)
        mainLayout.addLayout(submit)
        mainLayout.addLayout(recon)
        mainLayout.addLayout(data)
        self.setLayout(mainLayout)

        self.onlineSignal.connect(self.setOnline)
        self.conn = tcpConn.Connection(addr, port, self.writeback, self.onlineSignal, self.reconAuto.isChecked)

    def sendCommand(self):
        if self.connected:
            cmd = self.command.text()
            if cmd != "":
                self.writeback.emit(cmd, "#575757")
                self.command.setText("")
                self.conn.socket_write(cmd)

    def close(self, override):
        self.conn.close(override)
        self.connLog.logClose()

    def reconnect(self):
        if not self.connected:
            self.conn.reconnect()

    @Slot(bool, bool)
    def setOnline(self, active, connected):
        self.connected = connected
        self.buttonSend.setEnabled(connected)
        self.reconButton.setEnabled(not self.reconAuto.isChecked() and not active)

class commandLine(QLineEdit):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.history = [""]
        self.historyIndex = 0

    def keyPressEvent(self, event):
        key = event.key()
        if key == Qt.Key_Up:
            if self.historyIndex > 0:
                if self.historyIndex == len(self.history) - 1:
                    self.history[self.historyIndex] = self.text()
                self.historyIndex = self.historyIndex - 1
                self.setText(self.history[self.historyIndex])
        elif key == Qt.Key_Down:
            if self.historyIndex < len(self.history) - 1:
                self.historyIndex = self.historyIndex + 1
                self.setText(self.history[self.historyIndex])
        elif key == Qt.Key_Return:
            self.history[len(self.history) - 1] = self.text()
            self.historyIndex = len(self.history)
            self.history.append("")
            super().keyPressEvent(event)
        else:
            super().keyPressEvent(event)

class ConsoleLog(QTextEdit):
    def __init__(self, dataBuf, parent=None):
        super().__init__(parent)
        self.setReadOnly(True)
        self.setWordWrapMode(QTextOption.NoWrap)
        self.writeMutex = threading.Lock()
        self.data = []
        self.dataBuf = dataBuf
        self.log = False
        self.file = None
        self.fileData = None
        self.fileAll = None
        self.fileDataAll = None
        self.logOpen()
        
    def logOpen(self):
        try:
            timestamp = datetime.datetime.now().strftime("%d-%H-%M-%S")
            self.file = open("{}.log".format(date), "a")
        except:
            return False
        try:
            self.fileData = open("{}.data.log".format(date), "a")
        except:
            self.file.close()
            return False
        try:
            self.fileAll = open("client.log", "a")
        except:
            self.file.close()
            self.fileData.close()
            return False
        try:
            self.fileDataAll = open("client.data.log", "a")
        except:
            self.file.close()
            self.fileData.close()
            self.fileAll.close()
            return False
        self.log = True
        return True

    def logClose(self):
        if self.log:
            self.file.close()
            self.fileData.close()
            self.fileAll.close()
            self.fileDataAll.close()
            self.log = False

    @Slot(str, str)
    def addText(self, text, color):
        timestamp = datetime.datetime.now().strftime("%d-%H-%M-%S") + ": "
        self.writeMutex.acquire()
        if self.log:
            if text[:6] == "<data>":
                end = text.find("</data>")
                if end < 0: 
                    return
                data = text[6:end]
                self.fileData.write(data)
                self.fileDataAll.write(data)
                dataRaw = data.split(", ")
                if dataRaw[1] in self.dataBuf:
                    currTime = time.localTime(int(dataRaw[0])).strftime("%H:%M:%S")
                    self.dataBuf[dataRaw[1]].setText("{}: {} ({})".format(dataRaw[1], dataRaw[2], currTime))
                text = "DATA from {} at {}: {}".format(data[1], data[0], data[2])
                color = "#0000ff"
            else:
                self.file.write(timestamp + text + '\n')
                self.fileAll.write(timestamp + text + '\n')
        self.moveCursor(QTextCursor.End)
        if color == "":
            self.insertHtml(timestamp + text.replace('\n', '<br>') + '<br>')
        else:
            self.insertHtml(timestamp + '<font color="' + color + '">' + text.replace('\n', '<br>') + '</font><br>')
        self.moveCursor(QTextCursor.End)
        self.writeMutex.release()

class StartTab(QDialog):
    newConnection = Signal(str, str)
    def __init__(self, parent=None):
        global conn
        super().__init__(parent)
        self.parent = parent
        addrLabel = QLabel("Address:")
        self.addrEdit = QLineEdit("192.168.1.18")
        portLabel = QLabel("Port:")
        self.portEdit = QLineEdit("16778")

        buttonBox = QPushButton("Open")
        buttonBox.setFixedSize(100, 25)
        buttonBox.clicked.connect(self.submit)
        if parent != None:
            self.newConnection.connect(parent.newConnection)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(addrLabel)
        mainLayout.addWidget(self.addrEdit)
        mainLayout.addWidget(portLabel)
        mainLayout.addWidget(self.portEdit)
        mainLayout.addWidget(buttonBox)
        mainLayout.addStretch(1)
        self.setLayout(mainLayout)

    def submit(self):
        self.newConnection.emit(self.addrEdit.text(), self.portEdit.text())


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.resize(640, 480)
    window.show()
    sys.exit(app.exec_())
