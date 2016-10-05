#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <iomanip>

#include <experimental/filesystem>
#include <fstream>

extern "C" {
#include "darknet_wrapper.h"
}

int main(int argc, char **argv) {
    /*
    gpu_index = find_int_arg(argc, argv, "-i", 0);
    if (find_arg(argc, argv, "-nogpu")) {
        gpu_index = -1;
    }

    if (gpu_index >= 0) {
        cuda_set_device(gpu_index);
    }
*/
    if (argc != 6) {
        std::cout << "usage: " << argv[0] << " <cfg> <weights> <input video> <out bboxes> <out img dir>";
    }

    const auto cfg_fname        = std::string(argv[1]);
    const auto weights_fname    = std::string(argv[2]);
    const auto video_in_fname   = std::string(argv[3]);
    const auto out_bboxes_fname = std::string(argv[4]);
    const auto out_img_dir      = std::string(argv[5]);

    /*cv::VideoCapture cap(video_in_fname);
    unsigned int frame_pos = 0;
    if (cap.isOpened()) {
        while (cap.grab()) {
            cv::Mat frame;
            cap.retrieve(frame);

            frame_pos = static_cast<unsigned int>(cap.get(CV_CAP_PROP_POS_FRAMES));
            std::ostringstream format;
            format << out_img_dir << '/' << frame_pos << ".jpg";
            cv::imwrite(format.str(), frame);
        }
    }
    cap.release();*/

    darknet_wrapper_init(cfg_fname.c_str(), weights_fname.c_str(), video_in_fname.c_str(), out_bboxes_fname.c_str(), out_img_dir.c_str());

    return 0;
}