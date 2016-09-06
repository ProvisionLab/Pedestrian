#ifndef FRAMEREADER_H
#define FRAMEREADER_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class FrameReader
{
public:
    FrameReader();

    void initSource(std::string path);
    bool getFrame(cv::Mat& frame, bool &isNew);
    long getFrameNum();
    long getTotalNumFrames() const;

    void pause();
    void pause(bool pause);
    void next();

private:
    void scanDir(std::string path);
    void save();
    void load();

private:
    cv::VideoCapture videoCapture_;
    long frameNum;
    long totalNumFrames;
    std::string path;

    bool loadSingleFrame_;
    bool pause_;

    cv::Mat frame_;
};

#endif // FRAMEREADER_H
