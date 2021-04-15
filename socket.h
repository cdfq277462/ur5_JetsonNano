#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>

class socket : public QObject
{
    Q_OBJECT
public:
    explicit socket();
    //void run();

    void TCPsocketforUR5();

    bool getStatus();

    bool TCPstop();

    QByteArray RecvData();
signals:

public slots:
    void startConnecting();

    void connected_wirte();
    void connected_read();
    void disconnected_wirte();
    void disconnected_read();
    void bytesWriten(qint64);
    void readyRead();

    bool TCPStatus();

private:
    QTcpSocket *tcpsocket_write;
    QTcpSocket *tcpsocket_read;

    QByteArray receiveData;
};

#endif // SOCKET_H
