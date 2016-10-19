////////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
////////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************************************
USAGE:
./opencv_annotation -images <folder location> -annotations <ouput file>

Created by: Puttemans Steven - February 2015
Adapted by: Puttemans Steven - April 2016 - Vectorize the process to enable better processing
                                               + early leave and store by pressing an ESC key
                                               + enable delete `d` button, to remove last annotation
*****************************************************************************************************/

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>

// Function prototypes
void on_mouse(int, int, int, int, void*);
std::vector<cv::Rect> get_annotations(cv::Mat);

// Public parameters
cv::Mat image;
int roi_x0 = 0, roi_y0 = 0, roi_x1 = 0, roi_y1 = 0;
bool start_draw = false, stop = false;

// Window name for visualisation purposes
const std::string window_name = "OpenCV Based Annotation Tool";

// FUNCTION : Mouse response for selecting objects in images
// If left button is clicked, start drawing a rectangle as long as mouse moves
// Stop drawing once a new left click is detected by the on_mouse function
void on_mouse(int event, int x, int y, int , void * )
{
    // Action when left button is clicked
    if(event == cv::EVENT_LBUTTONDOWN)
    {
        if(!start_draw)
        {
            roi_x0 = x;
            roi_y0 = y;
            start_draw = true;
        } else {
            roi_x1 = x;
            roi_y1 = y;
            start_draw = false;
        }
    }

    // Action when mouse is moving and drawing is enabled
    if((event == cv::EVENT_MOUSEMOVE) && start_draw)
    {
        // Redraw bounding box for annotation
        cv::Mat current_view;
        image.copyTo(current_view);
        cv::rectangle(current_view, cv::Point(roi_x0,roi_y0), cv::Point(x,y), cv::Scalar(0,0,255));
        imshow(window_name, current_view);
    }
}

// FUNCTION : returns a vector of Rect objects given an image containing positive object instances
std::vector<cv::Rect> get_annotations(cv::Mat input_image)
{
    std::vector<cv::Rect> current_annotations;

    // Make it possible to exit the annotation process
    stop = false;

    // Init window interface and couple mouse actions
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(window_name, on_mouse);

    image = input_image;
    imshow(window_name, image);
    int key_pressed = 0;

    do
    {
        // Get a temporary image clone
        cv::Mat temp_image = input_image.clone();
        cv::Rect currentRect(0, 0, 0, 0);

        // Keys for processing
        // You need to select one for confirming a selection and one to continue to the next image
        // Based on the universal ASCII code of the keystroke: http://www.asciitable.com/
        //      <c> = 99          add rectangle to current image
        //	<n> = 110	  save added rectangles and show next image
        //      <d> = 100         delete the last annotation made
        //	<ESC> = 27        exit program
        key_pressed = 0xFF & cv::waitKey(0);
        switch( key_pressed )
        {
        case 27:
                stop = true;
                break;
        case 99:
                // Draw initiated from top left corner
                if(roi_x0<roi_x1 && roi_y0<roi_y1)
                {
                    currentRect.x = roi_x0;
                    currentRect.y = roi_y0;
                    currentRect.width = roi_x1-roi_x0;
                    currentRect.height = roi_y1-roi_y0;
                }
                // Draw initiated from bottom right corner
                if(roi_x0>roi_x1 && roi_y0>roi_y1)
                {
                    currentRect.x = roi_x1;
                    currentRect.y = roi_y1;
                    currentRect.width = roi_x0-roi_x1;
                    currentRect.height = roi_y0-roi_y1;
                }
                // Draw initiated from top right corner
                if(roi_x0>roi_x1 && roi_y0<roi_y1)
                {
                    currentRect.x = roi_x1;
                    currentRect.y = roi_y0;
                    currentRect.width = roi_x0-roi_x1;
                    currentRect.height = roi_y1-roi_y0;
                }
                // Draw initiated from bottom left corner
                if(roi_x0<roi_x1 && roi_y0>roi_y1)
                {
                    currentRect.x = roi_x0;
                    currentRect.y = roi_y1;
                    currentRect.width = roi_x1-roi_x0;
                    currentRect.height = roi_y0-roi_y1;
                }
                // Draw the rectangle on the canvas
                // Add the rectangle to the vector of annotations
                current_annotations.push_back(currentRect);
                break;
        case 100:
                // Remove the last annotation
                if(current_annotations.size() > 0)
                    current_annotations.pop_back();
                break;
        default:
                // Default case --> do nothing at all
                // Other keystrokes can simply be ignored
                break;
        }

        // Check if escape has been pressed
        if(stop)
        {
            break;
        }

        // Draw all the current rectangles onto the top image and make sure that the global image is linked
        for(int i=0; i < (int)current_annotations.size(); i++)
            rectangle(temp_image, current_annotations[i], cv::Scalar(0,255,0), 1);

        image = temp_image;

        // Force an explicit redraw of the canvas --> necessary to visualize delete correctly
        cv::imshow(window_name, image);
    }
    // Continue as long as the next image key has not been pressed
    while(key_pressed != 110);

    // Close down the window
    cv::destroyWindow(window_name);

    // Return the data
    return current_annotations;
}

int main( int argc, const char** argv )
{
    // If no arguments are given, then supply some information on how this tool works
    if( argc == 1 )
    {
        std::cout << "Usage: " << argv[0] << std::endl;
        std::cout << " -video <video file> [example - Leauto_20160829_164101A.MP4]" << std::endl;
        std::cout << " -annotations <ouput_file> [example - /data/annotations.txt]" << std::endl;
        std::cout << " -skip <frame skip> [example - 30]" << std::endl;
        std::cout << "TIP: Use absolute paths to avoid any problems with the software!" << std::endl;
        return -1;
    }

    // Read in the input arguments
    std::string video_file;
    std::string annotations_file;
    unsigned int frame_skip = 0;
    for(int i = 1; i < argc; ++i )
    {
        if(!strcmp( argv[i], "-video" ))
        {
            video_file = argv[++i];
        }
        else if(!strcmp( argv[i], "-annotations" ))
        {
            annotations_file = argv[++i];
        }
        else if(!strcmp( argv[i], "-skip" ))
        {
            std::stringstream str_stream;
            str_stream << argv[++i];
            str_stream >> frame_skip;
        }
    }



    // Start by processing the data
    std::map<int, std::vector<cv::Rect>> annotations;

    cv::VideoCapture cap(video_file);
    assert(cap.isOpened());

    int frame_count = 0;
    for(int frame_num = 0; cap.grab(); ++frame_num)
    {
        if(!frame_skip || !(frame_num % frame_skip))
        {
            cv::Mat current_image;
            bool ret = cap.retrieve(current_image);
            assert(ret && !current_image.empty());

            std::vector<cv::Rect> current_annotations = get_annotations(current_image);

            if(!current_annotations.empty())
            {
                assert(annotations.find(frame_num) == annotations.end());
                annotations.insert({frame_num, current_annotations});
            }

            ++frame_count;
        }
    }

    // When all data is processed, store the data gathered inside the proper file
    // This now even gets called when the ESC button was hit to store preliminary results
    std::ofstream output(annotations_file.c_str());
    if (!output.is_open())
    {
        std::cerr << "The path for the output file contains an error and could not be opened. Please check again!"
                  << std::endl;
        return 0;
    }


    output << frame_count << std::endl;
    for(auto && i : annotations)
        for(auto && j : i.second)
            output << i.first << " "
                   << j.x << " " << j.y << " "
                   << j.x + j.width << " " << j.y + j.height
                   << std::endl;

    cv::destroyAllWindows();
    return 0;
}
