#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QObject>
#include <QThread>
#include <librealsense2/rs.hpp>
#include <QImage>


#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/dnn.hpp"

using namespace std;

class cameraThread : public QThread
{
    Q_OBJECT
public:
    cameraThread();

protected:
    void run() override;
    void frameConfig(int rgb_width, int rgb_height, int depth_width, int depth_height, int fps);

    ~cameraThread() {}

public slots:
    // Member function that handles thread iteration

    void detectObjectsDNN(cv::Mat &frame);

    // If called it will stop the thread
    void stop() { isCameraRunning = false; }

    bool cameraStatus();

private:
    // Realsense configuration structure, it will define streams that need to be opened
    rs2::config cfg;

    // Our pipeline, main object used by realsense to handle streams
    rs2::pipeline pipe;

    // Frames returned by our pipeline, they will be packed in this structure
    rs2::frameset frames;

    // A bool that defines if our thread is running
    bool isCameraRunning = true;

    // object detection
    cv::CascadeClassifier *classifier;

    cv::dnn::Net net;
    vector<string> objectClasses;

signals:
    // A signal sent by our class to notify that there are frames that need to be processed
    //void framesReady(QImage frameRGB, QImage frameDepth);
    //void framesReady(QImage, QImage);
    void framesReady(QImage, QImage);


};
// A function that will convert realsense frames to QImage
QImage realsenseFrameToQImage(const rs2::frame& f);


#endif // CAMERATHREAD_H
