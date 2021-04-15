#include "socket.h"

#include <QThread>
#include <QAbstractSocket>
socket::socket()
{
    // read port: 30013
    // write port: 30003
    tcpsocket_write = new QTcpSocket(this);
    tcpsocket_read = new QTcpSocket(this);
}

void socket::startConnecting()
{
    //qDebug() << this->thread()->currentThreadId();
    TCPsocketforUR5();
}
void socket::TCPsocketforUR5()
{
    QString server_address = "192.168.2.9";
    int port_write = 30003;
    int port_read = 30013;
    //qDebug() << this->thread()->currentThreadId();


    connect(tcpsocket_write, SIGNAL(connected()), this, SLOT(connected_wirte()));
    //connect(tcpsocket_write, SIGNAL(disconnected()), this, SLOT(disconnected_wirte()));
    connect(tcpsocket_write, SIGNAL(bytesWritten(qint64)()), this, SLOT(bytesWriten(qint64)()));

    connect(tcpsocket_read, SIGNAL(connected()), this, SLOT(connected_read()));
    //connect(tcpsocket_read, SIGNAL(disconnected()), this, SLOT(disconnected_read()));
    connect(tcpsocket_read, SIGNAL(readyRead()), this, SLOT(readyRead()));
    //connect(tcpsocket_read, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(tcpstateread(QAbstractSocket::SocketState)));

    qDebug() << "Connecting to Write...";
    tcpsocket_write->connectToHost(server_address, port_write);

    if(!tcpsocket_write->waitForConnected(2000))
    {
        // fail to connect printf error info
        qDebug() << "Not Connected";
        qDebug() << "Error: " <<  tcpsocket_write->errorString();
    }
    // make sure W socket connected
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
    //qDebug() << getStatus();
}

bool socket::getStatus()
{
    return TCPStatus();
}


QByteArray socket::RecvData()
{
    return receiveData;
}

void socket::connected_wirte()
{
    qDebug() << "connected to wirte";
}
void socket::connected_read()
{
    qDebug() << "connected to read";
}
void socket::disconnected_wirte()
{
    qDebug() << "Disconnected";
    tcpsocket_write->disconnect();
}
void socket::disconnected_read()
{
    qDebug() << "Disconnected";
    tcpsocket_read->disconnect();
}
void socket::bytesWriten(qint64 bytes)
{

}
void socket::readyRead()
{
    //qDebug() << this->thread()->currentThreadId();
    receiveData = tcpsocket_read->readAll();
    //qDebug() << receiveData;
}

bool socket::TCPStatus()
{
    return (tcpsocket_write->state() == QAbstractSocket::ConnectedState  \
            || tcpsocket_read->state() == QAbstractSocket::ConnectedState);
}

