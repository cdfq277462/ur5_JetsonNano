#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "socket.h"
#include <librealsense2/rs.hpp>
#include "Camera.h"

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QAbstractSocket>
#include <QThread>
#include <QString>

double jointPos[6];
double ToolCenterPoint[6];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->frame_Home->raise();
    //qDebug() << this->thread()->currentThreadId();
    mSocket = new socket;
    updateDisplay_timeid = startTimer(100);
    qDebug() << mSocket->getStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::receiveFrame(QImage rgb)
{
    ui->label_rgb->setPixmap(QPixmap::fromImage(rgb));
    ui->label_depth->setPixmap(QPixmap::fromImage(rgb));
}

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

void MainWindow::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == updateDisplay_timeid)
    {
        if(mSocket->getStatus())
        {
            // status = connected
            ui->pushButton_connectRobot->setText("Disconnect");
            ui->pushButton_connectRobot->setEnabled(true);
            receiveData();
        }
        else
            ui->pushButton_connectRobot->setText("Connect");
    }
}


void MainWindow::on_pushButton_connectRobot_clicked()
{
    // change button state
    QString buttonStatus = ui->pushButton_connectRobot->text();
    QThread *tcpthread = new QThread;

    if(buttonStatus == "Connect")
    {
        ui->pushButton_connectRobot->setText("Connecting...");
        ui->pushButton_connectRobot->setEnabled(false);
        // connect
        // create new thread
        mSocket->moveToThread(tcpthread);
        connect(tcpthread, SIGNAL(started()), mSocket, SLOT(startConnecting()));
        tcpthread->start();
    }
    else
    {
        ui->pushButton_connectRobot->setText("Connect");
        ui->pushButton_connectRobot->setEnabled(true);

        //mSocket->disconnect();
        mSocket->disconnected_read();
        mSocket->disconnected_wirte();
    }
    //qDebug() << mSocket->getStatus();
}

void MainWindow::on_pushButton_connectCam_clicked()
{
    QThread *CamThread = new QThread;
    Camera *camera = new Camera;

    camera->frameConfig(640, 480, 320, 240, 30);

    camera->moveToThread(CamThread);
    // Connect the signal from the camera to the slot of the window
    // QApplication::connect(&camera, SIGNAL(framesReady(QImage, QImage)), this, SLOT(receiveFrame(QImage, QImage)));
    connect(CamThread, SIGNAL(started()), camera, SLOT(startCapture()));
    QApplication::connect(camera, SIGNAL(framesReady(QImage)), this, SLOT(receiveFrame(QImage)));

    CamThread->start();
}

void MainWindow::on_pushButton_connectGripper_clicked()
{

}

void MainWindow::on_pushButton_connectMag_clicked()
{
}

void MainWindow::requsetTCPstatus()
{
    // change button text
    if(mSocket->getStatus())
    {
        ui->pushButton_connectRobot->setText("Disconnect");
        ui->pushButton_connectRobot->setEnabled(true);
    }
    else
        ui->pushButton_connectRobot->setText("Connect");
}

void MainWindow::on_pushButton_home_clicked()
{
    ui->frame_Home->raise();
}
double MainWindow::HexToDouble(QByteArray array)
{
    int sign = 1;
    double exponent;
    double mantissa;

    // convert hex to binary
    array = QByteArray::number(array.toULongLong(0, 16), 2);
    // fill '0' to 64bits
    array = array.rightJustified(64, '0');
    //qDebug() << array << array.length();

    if(array.at(0) == '1')
        sign = -1;
    // exponent represented by 2's complement with 11 bits
    exponent = array.mid(1, 11).toInt(0 ,2) - 1023;

    QString fraction = array.right(52);   //get the fractional part
    for(int i = 0; i < fraction.length(); i++)
        if(fraction.at(i) == '1')
            mantissa += 1.0 / pow(2, i + 1);
    //qDebug() << sign << exponent << mantissa;
    return sign*pow(2,exponent)*(mantissa+1.0);
}
