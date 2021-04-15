#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "socket.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    double HexToDouble(QByteArray);
public slots:
    //void receiveFrame(QImage rgb, QImage depth);
    void receiveFrame(QImage rgb);
    void receiveData();

private slots:
    void timerEvent(QTimerEvent *event);

    void on_pushButton_connectRobot_clicked();

    void on_pushButton_connectCam_clicked();

    void on_pushButton_connectGripper_clicked();

    void on_pushButton_connectMag_clicked();

    void requsetTCPstatus();

    void on_pushButton_home_clicked();

private:
    Ui::MainWindow *ui;

    int updateDisplay_timeid;

    socket *mSocket;
};

#endif // MAINWINDOW_H
