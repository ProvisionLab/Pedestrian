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
    if (argc != 6) {
        std::cout << "usage: " << argv[0] << " <cfg> <weights> <input video> <out bboxes> <out img dir>";
    }

    const auto cfg_fname        = std::string(argv[1]);
    const auto weights_fname    = std::string(argv[2]);
    const auto video_in_fname   = std::string(argv[3]);
    const auto out_bboxes_fname = std::string(argv[4]);
    const auto out_img_dir      = std::string(argv[5]);

    darknet_wrapper_init(cfg_fname.c_str(), weights_fname.c_str(), video_in_fname.c_str(), out_bboxes_fname.c_str(), out_img_dir.c_str());

    return 0;
}