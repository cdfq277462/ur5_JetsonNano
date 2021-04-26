# Jetson Nano Develop UI with Qt for UR5
## Introduct
Use Qt to develop embedded system
- Socket connected to UR5
- Intel realsense D435 capture image
- 


### TCP Socket
Qt provide complete library for TCP or UDP socket.

In `ur5_JetsonNano.pro`
add line :
```
QT += network
```

Create new h/cpp file for TCP socket, here I named `socket.h` & `socket.cpp`.
In `socket.h`:
first, include `QTcpSocket`:
```cpp
#include <QTcpSocket>
```
declare function:
```cpp
public:
    socket();
    void TCPsocketforUR5();
    bool getStatus();
    bool TCPstop();
    QByteArray RecvData();
public slots:
    void startConnecting();     // start connecting
    void connected_wirte();     // connect to write
    void connected_read();      // connect to read
    void disconnected_wirte();
    void disconnected_read();
    void bytesWriten(qint64);   // used to comfirm how many bytes to write
    void readyRead();           // recevie data
    void SocketWrite(QString);  // write data
    bool TCPStatus();           // get connection status
private:
    QTcpSocket *tcpsocket_write;
    QTcpSocket *tcpsocket_read;
    QByteArray receiveData;
```
In `socket.cpp` :
first, create new tcp object and connect signals & slots :
```cpp
socket::socket()
{
    tcpsocket_write = new QTcpSocket(this);     // creat new tcpsocket for write
    tcpsocket_read = new QTcpSocket(this);      // creat new tcpsocket for read
    // connected tcp socket signals to slots
    connect(tcpsocket_write, SIGNAL(connected()), this, SLOT(connected_wirte()));
    connect(tcpsocket_write, SIGNAL(disconnected()), this, SLOT(disconnected_wirte()));
    connect(tcpsocket_write, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWriten(qint64)));

    connect(tcpsocket_read, SIGNAL(connected()), this, SLOT(connected_read()));
    connect(tcpsocket_read, SIGNAL(disconnected()), this, SLOT(disconnected_read()));
    connect(tcpsocket_read, SIGNAL(readyRead()), this, SLOT(readyRead()));
}
void socket::startConnecting()
{
    TCPsocketforUR5();
}
```
```cpp
void socket::TCPsocketforUR5()
{
    // read port: 30013
    // write port: 30003
    QString server_address = "192.168.2.9";
    int port_write = 30003;
    int port_read = 30013;

    qDebug() << "Connecting to Write...";
    tcpsocket_write->connectToHost(server_address, port_write);

    if(!tcpsocket_write->waitForConnected(2000))
    {
        // fail to connect printf error info
        qDebug() << "Not Connected";
        qDebug() << "Error: " <<  tcpsocket_write->errorString();
    }
    qDebug() << "Connecting to Read...";
    if(tcpsocket_write->state() == QAbstractSocket::ConnectedState){
        tcpsocket_read->connectToHost(server_address, port_read);
        if(!tcpsocket_read->waitForConnected(2000))
        {
            // fail to connect printf error info
            qDebug() << "Not Connected";
            qDebug() << "Error: " <<  tcpsocket_read->errorString();
        }
    }
}
```
```cpp
void socket::disconnected_wirte()
{
    qDebug() << "Disconnected";
    tcpsocket_write->disconnectFromHost();
    qDebug() << tcpsocket_write->state();

}
void socket::disconnected_read()
{
    qDebug() << "Disconnected";
    tcpsocket_read->disconnectFromHost();
    qDebug() << tcpsocket_read->state();
}
```
```cpp
void socket::readyRead()
{
    // read all data
    receiveData = tcpsocket_read->readAll();
}
void socket::SocketWrite(QString cmd)
{
    // socket command must end with "\n"
    cmd.append("\n");
    qDebug() << cmd.toLocal8Bit();
    tcpsocket_write->write(cmd.toLocal8Bit());
}
bool socket::TCPStatus()
{
    // return tcp status
    return ((tcpsocket_write->state() == QAbstractSocket::ConnectedState)  \
            && (tcpsocket_read->state() == QAbstractSocket::ConnectedState));
}
```

In `mainwindow.h`
first we included `socket.h`
```cpp
#include "socket.h"
```
```cpp
private slots:
    void receiveData();     // recevie ur5 via TCP data 

private:
    double HexToDouble(QByteArray);     // trans data from Hex to double
    socket *mSocket;
```
In `mainwindow.cpp`
```cpp
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mSocket = new socket;
}
```
```cpp
void MainWindow::receiveData()
{
    QByteArray RecvData = mSocket->RecvData();
    //qDebug() << RecvData;

    // 252 - 299
    // actual joint pos
    jointPos[0] = qRadiansToDegrees(HexToDouble(RecvData.mid(252, 8).toHex()));
    jointPos[1] = qRadiansToDegrees(HexToDouble(RecvData.mid(260, 8).toHex()));
    jointPos[2] = qRadiansToDegrees(HexToDouble(RecvData.mid(268, 8).toHex()));
    jointPos[3] = qRadiansToDegrees(HexToDouble(RecvData.mid(276, 8).toHex()));
    jointPos[4] = qRadiansToDegrees(HexToDouble(RecvData.mid(284, 8).toHex()));
    jointPos[5] = qRadiansToDegrees(HexToDouble(RecvData.mid(292, 8).toHex()));

    // 444 - 491
    // actual cartesian coordinates of the tool: (x, y, z, rx, ry, rz) = tcp
    ToolCenterPoint[0] = HexToDouble(RecvData.mid(444, 8).toHex());
    ToolCenterPoint[1] = HexToDouble(RecvData.mid(452, 8).toHex());
    ToolCenterPoint[2] = HexToDouble(RecvData.mid(460, 8).toHex());
    ToolCenterPoint[3] = HexToDouble(RecvData.mid(468, 8).toHex());
    ToolCenterPoint[4] = HexToDouble(RecvData.mid(476, 8).toHex());
    ToolCenterPoint[5] = HexToDouble(RecvData.mid(484, 8).toHex());
}
```
```cpp
double MainWindow::HexToDouble(QByteArray array)
{
    int sign = 1;
    double exponent;
    double mantissa;

    // convert hex to binary
    array = QByteArray::number(array.toULongLong(0, 16), 2);
    // fill '0' to 64bits
    array = array.rightJustified(64, '0');

    if(array.at(0) == '1')
        sign = -1;
    // exponent represented by 2's complement with 11 bits
    exponent = array.mid(1, 11).toInt(0 ,2) - 1023;

    QString fraction = array.right(52);   //get the fractional part
    for(int i = 0; i < fraction.length(); i++)
        if(fraction.at(i) == '1')
            mantissa += 1.0 / pow(2, i + 1);
    return sign*pow(2,exponent)*(mantissa+1.0);
}
```
