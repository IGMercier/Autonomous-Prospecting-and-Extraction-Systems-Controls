# This Python file uses the following encoding: utf-8
import sys, threading
from PySide2.QtWidgets import QApplication, QMainWindow, QLabel, QLineEdit, QWidget, QTabWidget, \
                              QVBoxLayout, QToolBar, QFrame, QDialog, QPushButton, QTextEdit, \
                              QHBoxLayout
from PySide2.QtGui  import QTextOption, QTextCursor
from PySide2.QtCore import Signal, Slot
from PySide2.QtCore import qDebug as qPrint
import tcpConn

class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        QMainWindow.__init__(self)
        self.tabBar = TabBar(self)
        self.setCentralWidget(self.tabBar)
        self.setWindowTitle("Tab Testing")

    def closeEvent(self, event):
        for i in range(1, len(self.tabBar.tabs)):
            self.tabBar.closeConn(i)
            print("Tab {} closed".format(i))
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
            self.tabs[index].close()
            self.removeTab(index)
            self.tabs.pop(index)

class ConnTab(QDialog):
    writeback = Signal(str)
    onlineState = Signal(bool, bool)
    def __init__(self, parent, addr, port):
        super().__init__(parent)
        self.parent = parent
        self.online = False
        self.connLog = ConsoleLog(self)
        self.writeback.connect(self.connLog.addText)
        self.writeback.emit("Connecting...")

        self.onlineState.connect(self.setOnline)

        self.command = QLineEdit()
        button = QPushButton("Send")
        self.command.returnPressed.connect(button.click)
        button.clicked.connect(self.sendCommand)
        button.setFixedSize(100, 25)
        submit = QHBoxLayout()
        submit.addWidget(self.command)
        submit.addWidget(button)
        submit.update()
        self.recon = QPushButton("Reconnect")
        self.recon.setEnabled(False)
        self.recon.clicked.connect(self.reconnect)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.connLog)
        mainLayout.addLayout(submit)
        mainLayout.addWidget(self.recon)
        mainLayout.addStretch(1)
        self.setLayout(mainLayout)

        self.conn = tcpConn.Connection(addr, port, self.writeback, self.onlineState)

    def sendCommand(self):
        if self.online:
            cmd = self.command.text()
            self.conn.socket_write(cmd + '\n')
            self.writeback.emit('<p style="color:#575757";>> ' + cmd + '</p>')
            self.command.setText("")

    def close(self):
        if self.online:
            self.writeback.emit('<p style="color:#ff0000";>Connection Closed.</p>')
            self.conn.close()

    def reconnect(self):
        self.writeback.emit("Connecting...")
        self.conn.reconnect()


    @Slot(bool)
    def setOnline(self, online, connected):
        self.online = online
        self.recon.setEnabled(not connected)


class ConsoleLog(QTextEdit):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setReadOnly(True)
        self.setWordWrapMode(QTextOption.NoWrap)
        self.writeMutex = threading.Lock()

    @Slot(str)
    def addText(self, text):
        self.writeMutex.acquire()
        self.moveCursor(QTextCursor.End)
        self.insertHtml(text + '<br>')
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
