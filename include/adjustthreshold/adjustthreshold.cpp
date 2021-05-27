#include "adjustthreshold.h"

Adjustthreshold::Adjustthreshold()
{
}

bool Adjustthreshold::readImg(std::string imgName)
{
    origImg = cv::imread(imgName);
    if(origImg.empty())
        return false;
    cv::cvtColor(origImg, grayImg, cv::COLOR_BGR2GRAY);

    return true;
}

double Adjustthreshold::getAdjustThreshold(int whichThresholdMethod, double thresholdError)
{
    // calculate Mean
    cv::Scalar tmp_mean;
    tmp_mean = cv::mean(grayImg);
    // qDebug() << tmp_mean.val[0] /255;

    double threshold = tmp_mean.val[0];
    double thresholdNext = 0;
    bool done = (abs(threshold - thresholdNext) < thresholdError);

    while(!done)
    {
        cv::Scalar tmp_mean1, tmp_stddev1, tmp_mean2, tmp_stddev2;

        // calculate two group' mean & std except 0
        cv::meanStdDev(grayImg, tmp_mean1, tmp_stddev1, grayImg > threshold);
        cv::meanStdDev(grayImg, tmp_mean2, tmp_stddev2, grayImg < threshold);

        if(whichThresholdMethod != THRESHOLD_OTSU)
            switch (whichThresholdMethod) {
            case THRESHOLD_MEAN:
                thresholdNext = (tmp_mean1.val[0] + tmp_mean2.val[0]) /2;
                break;
            case THRESHOLD_MEANstd:
                thresholdNext = (1/ (tmp_stddev1.val[0] + tmp_stddev2.val[0]))  \
                                * (tmp_stddev1.val[0] * tmp_mean1.val[0] + tmp_stddev2.val[0] * tmp_mean2.val[0]);
                break;
            }
            // Tnext = (1/(s1+s2))*(s1*mean(fd(gd)) + s2*mean(fd(~gd)));
            // Tnext = (0.5)*(mean(fd(gd)) + mean(fd(~gd)));
        else
        {
            cv::Mat tmp_Mat;
            threshold = cv::threshold(grayImg, tmp_Mat, 0, 255, cv::THRESH_OTSU);
            break;
        }
        done = (abs(threshold - thresholdNext) < thresholdError);
        threshold = thresholdNext;
    }
    // qDebug() << threshold /255;
    return threshold;
}

cv::Mat Adjustthreshold::kcircle(int kCircleRadius)
{
    int kernelSize = kCircleRadius *2 + 1;
       cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(kernelSize, kernelSize));

       int kernelRows = kernel.rows;
       int kernelCols = kernel.cols;

       for(int j = 0; j < kernelCols; j++)
           for(int i = 0; i < kernelRows; i++)
           {
               int colDistance = qAbs(j - kCircleRadius);
               int rowDistance = qAbs(i - kCircleRadius);

               if((colDistance + rowDistance) <= kCircleRadius)
                   *kernel.ptr(j, i) = 1;
               else
                   *kernel.ptr(j, i) = 0;
           }
       return kernel;
}

cv::Mat Adjustthreshold::morphologyClosingOpening(cv::Mat src_Mat, int kCircleRadius)
{
    cv::Mat dst_Mat(src_Mat.rows, src_Mat.cols, src_Mat.type());
    // create kernel for dilation
    cv::Mat kernel = kcircle(kCircleRadius);

    // closing operate
    cv::morphologyEx(src_Mat, dst_Mat, cv::MORPH_CLOSE, kernel);
    // opening operate
    cv::morphologyEx(dst_Mat, dst_Mat, cv::MORPH_OPEN, kernel);

    return dst_Mat;
}
