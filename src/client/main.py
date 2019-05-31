# This Python file uses the following encoding: utf-8
import sys, threading
from PySide2.QtWidgets import QApplication, QMainWindow, QLabel, QLineEdit, QWidget, QTabWidget, \
                              QVBoxLayout, QToolBar, QFrame, QDialog, QPushButton, QTextEdit, \
                              QHBoxLayout
from PySide2.QtGui  import QTextOption, QTextCursor
from PySide2.QtCore import Signal
from PySide2.QtCore import qDebug as qPrint
import tcpConn

conn = 0

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

    def newConnection(self, name, addr, port):
        newConn = ConnTab(self, addr, int(port))
        self.tabs.append(newConn)
        self.addTab(newConn, name)
        self.setCurrentWidget(newConn)

    def closeConn(self, index):
        if index != 0:
            self.tabs[index].close()
            self.removeTab(index)
            self.tabs.pop(index)

class ConnTab(QDialog):
    def __init__(self, parent, addr, port):
        super().__init__(parent)
        self.parent = parent
        self.online = False
        self.connLog = QTextEdit()
        self.connLog.setReadOnly(True)
        self.connLog.setPlainText("Connecting...\n")
        self.connLog.setWordWrapMode(QTextOption.NoWrap)

        self.command = QLineEdit()
        button = QPushButton("Send")
        self.command.returnPressed.connect(button.click)
        button.clicked.connect(self.sendCommand)
        button.setFixedSize(100, 25)
        submit = QHBoxLayout()
        submit.addWidget(self.command)
        submit.addWidget(button)
        submit.update()

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.connLog)
        mainLayout.addLayout(submit)
        mainLayout.addStretch(1)
        self.setLayout(mainLayout)

        self.writeMutex = threading.Lock()
        self.conn = tcpConn.Connection(addr, port, self)

    def sendCommand(self):
        if self.conn.online:
            cmd = self.command.text()
            self.conn.socket_write(cmd + '\n')
            self.addText('<p style="color:#575757";>> ' + cmd + '</p>')
            self.command.setText("")

    def addText(self, text):
        self.writeMutex.acquire()
        self.connLog.moveCursor(QTextCursor.End)
        self.connLog.insertHtml(text + '<br>')
        self.connLog.moveCursor(QTextCursor.End)
        self.writeMutex.release()

    def close(self):
        if self.conn.online:
            self.addText('<p style="color:#ff0000";>Connection Closed.</p>')
            self.conn.close_external()

class StartTab(QDialog):
    newConnection = Signal(str, str, str)
    def __init__(self, parent=None):
        global conn
        super().__init__(parent)
        self.parent = parent
        conn = conn + 1
        current = conn
        connLabel = QLabel("Name:")
        self.connEdit = QLineEdit("Connection {}".format(conn))
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
        mainLayout.addWidget(connLabel)
        mainLayout.addWidget(self.connEdit)
        mainLayout.addWidget(addrLabel)
        mainLayout.addWidget(self.addrEdit)
        mainLayout.addWidget(portLabel)
        mainLayout.addWidget(self.portEdit)
        mainLayout.addWidget(buttonBox)
        mainLayout.addStretch(1)
        self.setLayout(mainLayout)

    def submit(self):
        global conn
        self.newConnection.emit(self.connEdit.text(), self.addrEdit.text(), self.portEdit.text())
        conn = conn + 1
        self.connEdit.setText("Connection {}".format(conn))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.resize(640, 480)
    window.show()
    sys.exit(app.exec_())
