#include "framereader.h"
#include <iostream>
#include <fstream>
#include <QDir>
#include <QMessageBox>

FrameReader::FrameReader() :
    frameNum(0),
    totalNumFrames(0),
    pause_(true),
    loadSingleFrame_(true)
{
}


void FrameReader::initSource(std::string path)
{
    videoCapture_.open(path);
    this->path = path;
    frameNum = 0;
    totalNumFrames = videoCapture_.get(CV_CAP_PROP_FRAME_COUNT);
    loadSingleFrame_ = true;
}

bool FrameReader::getFrame(cv::Mat& frame, bool& isNew)
{
    if (!pause_ || loadSingleFrame_)
    {
        loadSingleFrame_ = false;

        for (int i = 0; i < 200; i++)
        {
            if (frameNum  == totalNumFrames)
            {
                return false;
            }
            frameNum++;
            videoCapture_ >> frame_;
            if (frame_.empty())
            {
                return false;
            }
        }
        isNew = true;
    }
    else
    {
        isNew = false;
    }


//    if (frameNum == 10)
//    {
//        return false;
//    }

    frame_.copyTo(frame);

    return true;
}

long FrameReader::getFrameNum()
{
    return frameNum;
}

long FrameReader::getTotalNumFrames() const
{
    return totalNumFrames;
}

void FrameReader::pause()
{
    pause_ = !pause_;
}

void FrameReader::pause(bool pause)
{
    pause_ = pause;
}

void FrameReader::next()
{
    loadSingleFrame_ = true;
}
