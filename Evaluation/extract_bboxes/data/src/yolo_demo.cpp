/*#include "constants.h"
#include "network.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "utils.h"
#include "parser.h"
#include <sys/time.h>
#include "opencv2/highgui/highgui_c.h"

#define FRAMES 3

image get_image_from_stream(CvCapture *cap);

static char **demo_names;
static image *demo_alphabet;
static int demo_classes;

static float **probs;
static box *boxes;
static network net;
static image in   ;
static image in_s ;
static image det  ;
static image det_s;
static image disp = {0};
static CvCapture * cap;
static float fps = 0;
static float demo_thresh = 0;

static float *predictions[FRAMES];
static int demo_index = 0;
static image images[FRAMES];
static float *avg;

void *fetch_in_thread(void *ptr)
{
    in = get_image_from_stream(cap);
    if(!in.data){
        error("Stream closed.");
    }
    in_s = resize_image(in, net.w, net.h);
    return 0;
}

void *detect_in_thread(void *ptr)
{
    float nms = .4;

    layer l = net.layers[net.n-1];
    float *X = det_s.data;
    float *prediction = network_predict(net, X);

    memcpy(predictions[demo_index], prediction, l.outputs*sizeof(float));
    mean_arrays(predictions, FRAMES, l.outputs, avg);
    l.output = avg;

    free_image(det_s);
    if(l.type == DETECTION){
        get_detection_boxes(l, 1, 1, demo_thresh, probs, boxes, 0);
    } else if (l.type == REGION){
        get_region_boxes(l, 1, 1, demo_thresh, probs, boxes, 0);
    } else {
        error("Last layer must produce detections\n");
    }
    if (nms > 0) do_nms(boxes, probs, l.w*l.h*l.n, l.classes, nms);
    printf("\033[2J");
    printf("\033[1;1H");
    printf("\nFPS:%.1f\n",fps);
    printf("Objects:\n\n");

    images[demo_index] = det;
    det = images[(demo_index + FRAMES/2 + 1)%FRAMES];
    demo_index = (demo_index + 1)%FRAMES;

    draw_detections(det, l.w*l.h*l.n, demo_thresh, boxes, probs, demo_names, demo_alphabet, demo_classes);

    return 0;
}

double get_wall_time()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void demo(char *cfgfile, char *weightfile, float thresh, const char *filename, char **names, int classes, int frame_skip, char *prefix)
{
    //skip = frame_skip;
    image *alphabet = load_alphabet();
    int delay = frame_skip;
    demo_names = names;
    demo_alphabet = alphabet;
    demo_classes = classes;
    demo_thresh = thresh;
    printf("Demo\n");
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);

    srand(2222222);

    cap = cvCaptureFromFile(filename);

    if(!cap) error("Couldn't read from file.\n");

    layer l = net.layers[net.n-1];
    int j;

    avg = (float *) calloc(l.outputs, sizeof(float));
    for(j = 0; j < FRAMES; ++j) predictions[j] = (float *) calloc(l.outputs, sizeof(float));
    for(j = 0; j < FRAMES; ++j) images[j] = make_image(1,1,3);

    boxes = (box *)calloc(l.w*l.h*l.n, sizeof(box));
    probs = (float **)calloc(l.w*l.h*l.n, sizeof(float *));
    for(j = 0; j < l.w*l.h*l.n; ++j) probs[j] = (float *)calloc(l.classes, sizeof(float *));

    pthread_t fetch_thread;
    pthread_t detect_thread;

    fetch_in_thread(0);
    det = in;
    det_s = in_s;

    fetch_in_thread(0);
    detect_in_thread(0);
    disp = det;
    det = in;
    det_s = in_s;

    for(j = 0; j < FRAMES/2; ++j){
        fetch_in_thread(0);
        detect_in_thread(0);
        disp = det;
        det = in;
        det_s = in_s;
    }

    int count = 0;
    if(!prefix){
        cvNamedWindow("Demo", CV_WINDOW_NORMAL);
        cvMoveWindow("Demo", 0, 0);
        cvResizeWindow("Demo", 640, 480);
    }

    double before = get_wall_time();

    while(1){
        ++count;
        if(1){
            if(pthread_create(&fetch_thread, 0, fetch_in_thread, 0)) error("Thread creation failed");
            if(pthread_create(&detect_thread, 0, detect_in_thread, 0)) error("Thread creation failed");

            if(!prefix){
                show_image(disp, "Demo");
                int c = cvWaitKey(1);
                if (c == 10){
                    if(frame_skip == 0) frame_skip = 60;
                    else if(frame_skip == 4) frame_skip = 0;
                    else if(frame_skip == 60) frame_skip = 4;
                    else frame_skip = 0;
                }
            }else{
                char buff[256];
                sprintf(buff, "%s_%08d", prefix, count);
                save_image(disp, buff);
            }

            pthread_join(fetch_thread, 0);
            pthread_join(detect_thread, 0);

            if(delay == 0){
                free_image(disp);
                disp  = det;
            }
            det   = in;
            det_s = in_s;
        }
        else {
            fetch_in_thread(0);
            det   = in;
            det_s = in_s;
            detect_in_thread(0);
            if(delay == 0) {
                free_image(disp);
                disp = det;
            }
            show_image(disp, "Demo");
            cvWaitKey(1);
        }
        --delay;
        if(delay < 0){
            delay = frame_skip;

            double after = get_wall_time();
            float curr = 1./(after - before);
            fps = curr;
            before = after;
        }
    }
}

int main(int argc, char **argv) {
    gpu_index = find_int_arg(argc, argv, "-i", 0);
    if (find_arg(argc, argv, "-nogpu")) {
        gpu_index = -1;
    }

#ifndef GPU
    gpu_index = -1;
#else
    if (gpu_index >= 0) {
        cuda_set_device(gpu_index);
    }
#endif

    char *prefix = find_char_arg(argc, argv, "-prefix", 0);
    //float thresh = find_float_arg(argc, argv, "-thresh", .2);
    float thresh = 0.09;
    int frame_skip = find_int_arg(argc, argv, "-s", 0);

    char *cfg = argv[1];
    char *weights = argv[2];
    char *filename = argv[3];
    char *voc_names[] = CLASS_NAMES;
    demo(cfg, weights, thresh, filename, voc_names, NUM_CLASSES, frame_skip, prefix);

    return 0;
}
*/

#include <string>
#include <opencv2/opencv.hpp>

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

    darknet_wrapper_init(argc, argv);
    return 0;
    const std::string video_file = "video_in.seq";
    const std::string weight_file = "cpuNet-4c_final.weights";
    const std::string cfg_file = "cpuNet-4c.cfg";

    const int frame_skip = 0;
    const float thresh = .2f;

    const int delay = frame_skip;

    cv::VideoCapture cap(video_file);
    if (cap.isOpened()) {
        while (cap.grab()) {
            cv::Mat frame;
            cap.retrieve(frame);
            cv::imshow("video output", frame);
            if (cv::waitKey(1) >= 0)
                break;
        }
    }
    return 0;
}