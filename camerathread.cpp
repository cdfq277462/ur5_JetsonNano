#ifdef _WIN32
#define OPENCV
#define GPU
#endif

#include "camerathread.h"
#include "include/adjustthreshold.h"
//#include "include/yolo_v2_class.hpp"
//#pragma comment(lib, "include/yolo_cpp_dll.lib")  // imported DLL

#include <QtDebug>
#include <fstream>
#include <sstream>
#include <iostream>
cameraThread::cameraThread()
{

}

void cameraThread::run()
{
    // Enable depth stream with given resolution. Pixel will have a bit depth of 16 bit
    cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 15);

    // Enable RGB stream as frames with 3 channel of 8 bit
    cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 15);
    //cfg.enable_stream(RS2_STREAM_COLOR);
    pipe.start(cfg);
    isCameraRunning = true;


    //frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    //frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    while(isCameraRunning)
    {
        // Wait for frames and get them as soon as they are ready
        frames = pipe.wait_for_frames();


        // Let's get our depth frame
        rs2::depth_frame depth = frames.get_depth_frame();
        // And our rgb frame
        rs2::frame rgb = frames.get_color_frame();

        // Let's convert them to QImage
        auto q_rgb = realsenseFrameToQImage(rgb);
        auto q_depth = realsenseFrameToQImage(depth);

        int w = 640;
        int h = 480;
        cv::Mat color_image(cv::Size(w, h), CV_8UC3, (void*)rgb.get_data(), cv::Mat::AUTO_STEP); // 彩色影像

#ifdef TIME_MEASURE
        int64 t0 = cv::getTickCount();
#endif
        Adjustthreshold adjustThreshold;
        cv::Mat grayImg;
        cv::cvtColor(color_image, grayImg, cv::COLOR_BGR2GRAY);
        double threshold = adjustThreshold.getAdjustThreshold(grayImg, THRESHOLD_OTSU, 0.255);
        cv::threshold(grayImg, grayImg, threshold, 255, cv::THRESH_BINARY);
        grayImg = adjustThreshold.morphologyClosingOpening(grayImg, 3);
        //detectObjectsDNN(color_image);

#ifdef TIME_MEASURE
        int64 t1 = cv::getTickCount();
        double t = (t1-t0) * 1000 /cv::getTickFrequency();
        qDebug() << "Detecting time on a single frame: " << t <<"ms";

#endif

        QImage q_gray_image(grayImg.data, grayImg.cols, grayImg.rows, grayImg.step, QImage::Format_Grayscale8);
        QImage q_color_image(color_image.data, color_image.cols, color_image.rows, color_image.step, QImage::Format_RGB888);
        // And finally we'll emit our signal
        //emit framesReady(q_rgb);
        emit framesReady(q_color_image, q_gray_image);
        //emit framesReady(q_rgb, q_depth);
    }
}

void cameraThread::frameConfig(int rgb_width, int rgb_height, int depth_width, int depth_height, int fps)
{
    // Enable depth stream with given resolution. Pixel will have a bit depth of 16 bit
    cfg.enable_stream(RS2_STREAM_DEPTH, depth_width, depth_height, RS2_FORMAT_Z16, fps);

    // Enable RGB stream as frames with 3 channel of 8 bit
    cfg.enable_stream(RS2_STREAM_COLOR, rgb_width, rgb_height, RS2_FORMAT_RGB8, fps);
    //cfg.enable_stream(RS2_STREAM_COLOR);
    pipe.start(cfg);
    isCameraRunning = true;
}


bool cameraThread::cameraStatus()
{
    return isCameraRunning;
}

QImage realsenseFrameToQImage(const rs2::frame &f)
{
    using namespace rs2;

    auto vf = f.as<video_frame>();
    const int w = vf.get_width();
    const int h = vf.get_height();

    if (f.get_profile().format() == RS2_FORMAT_RGB8)
    {
        auto r = QImage((uchar*) f.get_data(), w, h, w*3, QImage::Format_RGB888);
        return r;
    }

    else if (f.get_profile().format() == RS2_FORMAT_Z16)
    {
        // only if you have Qt > 5.13
        auto r = QImage((uchar*) f.get_data(), w, h, w*2, QImage::Format_Grayscale16);
        return r;
    }


    throw std::runtime_error("Frame format is not supported yet!");
}

static vector<string> getOutputsNames(const cv::dnn::Net& net);
static void decodeOutLayers(
    cv::Mat &frame, const vector<cv::Mat> &outs,
    vector<int> &outClassIds,
    vector<float> &outConfidences,
    vector<cv::Rect> &outBoxes
);

void cameraThread::detectObjectsDNN(cv::Mat &frame)
{
    int inputWidth = 416;
    int inputHeight = 416;

    if (net.empty()) {
        // give the configuration and weight files for the model
        //string modelConfig = "data/yolov3.cfg";
        //string modelWeights = "data/yolov3.weights";
        string modelConfig = "data/yolov3-tiny.cfg";
        string modelWeights = "data/yolov3-tiny_last.weights";

        net = cv::dnn::readNetFromDarknet(modelConfig, modelWeights);
        // net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        // net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

        objectClasses.clear();
        string name;
        //string namesFile = "data/coco.names";
        string namesFile = "data/obj.names";
        ifstream ifs(namesFile.c_str());
        while(getline(ifs, name)) objectClasses.push_back(name);
    }

    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(inputWidth, inputHeight), cv::Scalar(0, 0, 0), true, false);

    net.setInput(blob);

    // forward
    vector<cv::Mat> outs;
    net.forward(outs, getOutputsNames(net));

#ifdef TIME_MEASURE
    vector<double> layersTimes;
    double freq = cv::getTickFrequency() / 1000;
    double t = net.getPerfProfile(layersTimes) / freq;
    qDebug() << "YOLO: Inference time on a single frame: " << t <<"ms";
    string label = cv::format("Inference time for a frame : %.2f ms", t);
    cv::putText(frame, label, cv::Point(0, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
#endif

    // remove the bounding boxes with low confidence
    vector<int> outClassIds;
    vector<float> outConfidences;
    vector<cv::Rect> outBoxes;
    decodeOutLayers(frame, outs, outClassIds, outConfidences, outBoxes);

    for(size_t i = 0; i < outClassIds.size(); i ++) {
        cv::rectangle(frame, outBoxes[i], cv::Scalar(0, 0, 255));

        // get the label for the class name and its confidence
        string label = objectClasses[outClassIds[i]];
        label += cv::format(":%.2f", outConfidences[i]);

        // display the label at the top of the bounding box
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        int left = outBoxes[i].x, top = outBoxes[i].y;
        top = max(top, labelSize.height);
        cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255));
    }

}

vector<string> getOutputsNames(const cv::dnn::Net& net)
{
    static vector<string> names;
    vector<int> outLayers = net.getUnconnectedOutLayers();
    vector<string> layersNames = net.getLayerNames();
    names.resize(outLayers.size());
    for (size_t i = 0; i < outLayers.size(); ++i)
        names[i] = layersNames[outLayers[i] - 1];

    return names;
}

void decodeOutLayers(
    cv::Mat &frame, const vector<cv::Mat> &outs,    \
    vector<int> &outClassIds,                       \
    vector<float> &outConfidences,                  \
    vector<cv::Rect> &outBoxes                      \
)
{
    float confThreshold = 0.5; // confidence threshold
    float nmsThreshold = 0.4;  // non-maximum suppression threshold

    vector<int> classIds;
    vector<float> confidences;
    vector<cv::Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i) {
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            // get the value and location of the maximum score
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold)
            {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    // non maximum suppression
    vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        outClassIds.push_back(classIds[idx]);
        outBoxes.push_back(boxes[idx]);
        outConfidences.push_back(confidences[idx]);
    }
}
