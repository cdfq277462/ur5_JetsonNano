#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QThread>
#include <librealsense2/rs.hpp>
#include <QImage>

class Camera : public QObject
{
    Q_OBJECT
public:
    Camera();
    //    Camera(int rgb_width, int rgb_height, int depth_width, int depth_height, int fps);

    void frameConfig(int rgb_width, int rgb_height, int depth_width, int depth_height, int fps);

    ~Camera() {}

public slots:
    // Member function that handles thread iteration
    void startCapture();

    // If called it will stop the thread
    void stop() { camera_running = false; }

    bool cameraStatus();

private:
    // Realsense configuration structure, it will define streams that need to be opened
    rs2::config cfg;

    // Our pipeline, main object used by realsense to handle streams
    rs2::pipeline pipe;

    // Frames returned by our pipeline, they will be packed in this structure
    rs2::frameset frames;

    // A bool that defines if our thread is running
    bool camera_running = true;

signals:
    // A signal sent by our class to notify that there are frames that need to be processed
    //void framesReady(QImage frameRGB, QImage frameDepth);
    void framesReady(QImage frameRGB);

};
// A function that will convert realsense frames to QImage
QImage realsenseFrameToQImage(const rs2::frame& f);

#endif // CAMERA_H
