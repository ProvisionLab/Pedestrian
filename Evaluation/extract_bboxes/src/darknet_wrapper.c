#include "darknet_wrapper.h"
#include "network.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "utils.h"
#include "parser.h"
#include "constants.h"
#include "image.h"
#include <sys/time.h>
#include <opencv2/highgui/highgui_c.h>

image get_image_from_stream(CvCapture *cap);

#define FRAMES 3

static char **demo_names;
static image *demo_alphabet;
static int demo_classes;

static float **probs;
static box *boxes;
static network net;
static image in   ;
static image in_s ;
static image det  ;
static image_pos *det_pos;
static int size_det_pos;
static image det_s;
static image disp = {0};
static CvCapture * cap;
static float fps = 0;
static float demo_thresh = 0;

static float *predictions[FRAMES];
static int demo_index = 0;
static image images[FRAMES];
static float *avg;

char *voc_names_demo[] = CLASS_NAMES;

void *fetch_in_thread() {
    in = get_image_from_stream(cap);
    if(!in.data) {
        error("Stream closed.");
    }
    in_s = resize_image(in, net.w, net.h);
    return 0;
}

void *detect_in_thread() {
    float nms = .4;

    layer l = net.layers[net.n-1];
    float *X = det_s.data;
    float *prediction = network_predict(net, X);

    memcpy(predictions[demo_index], prediction, l.outputs*sizeof(float));
    mean_arrays(predictions, FRAMES, l.outputs, avg);
    l.output = avg;

    free_image(det_s);
    if(l.type == DETECTION) {
        get_detection_boxes(l, 1, 1, demo_thresh, probs, boxes, 0);
    }
    else if (l.type == REGION) {
        get_region_boxes(l, 1, 1, demo_thresh, probs, boxes, 0);
    }
    else {
        error("Last layer must produce detections\n");
    }
    if (nms > 0)
        do_nms(boxes, probs, l.w*l.h*l.n, l.classes, nms);

    printf("\033[2J");
    printf("\033[1;1H");
    printf("\nFPS:%.1f\n",fps);
    printf("Objects:\n\n");

    images[demo_index] = det;
    det = images[(demo_index + FRAMES/2 + 1)%FRAMES];
    demo_index = (demo_index + 1)%FRAMES;

    int num_pos = l.w*l.h*l.n;
    //allocate det_pos
    int a1 = sizeof(*det_pos);
    int a2 = sizeof(image_pos);

    if (det_pos)
        free(det_pos);

    det_pos = (image_pos *)calloc(num_pos, sizeof(*det_pos));
    int a3 = 200;

    size_det_pos = 0;
    //draw_detections(det, l.w*l.h*l.n, demo_thresh, boxes, probs, demo_names, demo_alphabet, demo_classes);
    get_detection_bboxes(det, num_pos, demo_thresh, boxes, probs, demo_names, demo_alphabet, demo_classes, det_pos, &size_det_pos);
    for (int i = 0; i < size_det_pos; ++i) {
        int left = det_pos[0].left;
        int top = det_pos[0].top;
        int right = det_pos[0].right;
        int bottom = det_pos[0].bottom;
    }

    if (!size_det_pos && det_pos) {
        free(det_pos);
        det_pos = NULL;
    }

    return 0;
}

double get_wall_time()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)) {
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void demo(char *cfgfile, char *weightfile, float thresh, const char *filename, char **names, int classes, int frame_skip, char *prefix) {
    int delay = frame_skip;
    demo_names = names;
    demo_classes = classes;
    demo_thresh = thresh;
    printf("Demo\n");
    net = parse_network_cfg(cfgfile);
    load_weights(&net, weightfile);
    set_batch_network(&net, 1);

    srand(2222222);

    cap = cvCaptureFromFile(filename);

    if(!cap)
        error("Couldn't connect to webcam.\n");

    layer l = net.layers[net.n-1];
    int j;

    avg = (float *) calloc(l.outputs, sizeof(float));
    for(j = 0; j < FRAMES; ++j)
        predictions[j] = (float *) calloc(l.outputs, sizeof(float));
    for(j = 0; j < FRAMES; ++j)
        images[j] = make_image(1,1,3);

    boxes = (box *)calloc(l.w*l.h*l.n, sizeof(box));
    probs = (float **)calloc(l.w*l.h*l.n, sizeof(float *));
    for(j = 0; j < l.w*l.h*l.n; ++j)
        probs[j] = (float *)calloc(l.classes, sizeof(float *));

    pthread_t fetch_thread;
    pthread_t detect_thread;

    fetch_in_thread();
    det = in;
    det_s = in_s;

    for(j = 0; j < FRAMES/2 + 1; ++j){
        fetch_in_thread();
        detect_in_thread();
        disp = det;
        det = in;
        det_s = in_s;
    }

    int count = 0;
    if(!prefix) {
        cvNamedWindow("Demo", CV_WINDOW_NORMAL);
        cvMoveWindow("Demo", 0, 0);
        cvResizeWindow("Demo", 1352, 1013);
    }

    FILE *file_bboxes = fopen("bboxes.txt", "a");
    double before = get_wall_time();

    while (1) {
        ++count;
        if(pthread_create(&fetch_thread, 0, fetch_in_thread, 0))
            error("Thread creation failed");

        if(pthread_create(&detect_thread, 0, detect_in_thread, 0))
            error("Thread creation failed");

        if(!prefix){
            show_image(disp, "Demo");
            int c = cvWaitKey(1);
            if (c == 10) {
                if (frame_skip == 0)
                    frame_skip = 60;
                else if(frame_skip == 4)
                    frame_skip = 0;
                else if(frame_skip == 60)
                    frame_skip = 4;
                else
                    frame_skip = 0;
            }
        }
        else {
            char buff[256];
            sprintf(buff, "%s_%08d", prefix, count);
            save_image(disp, buff);

            if (file_bboxes == NULL) {
                printf("Error opening file!\n");
            }
            for (int p = 0; p < size_det_pos; ++p) {
                fprintf(file_bboxes, "%d %d %d %d %d\n", count, det_pos[p].left, det_pos[p].top, det_pos[p].right, det_pos[p].bottom);
            }
        }

        pthread_join(fetch_thread, 0);
        pthread_join(detect_thread, 0);

        if(delay == 0) {
            free_image(disp);
            disp  = det;
        }
        det   = in;
        det_s = in_s;

        --delay;
        if(delay < 0){
            delay = frame_skip;

            double after = get_wall_time();
            float curr = 1./(after - before);
            fps = curr;
            before = after;
        }
    }
    fclose(file_bboxes);
}

void darknet_wrapper_init(int argc, char **argv)
{
    gpu_index = find_int_arg(argc, argv, "-i", 0);
    if (find_arg(argc, argv, "-nogpu")) {
        gpu_index = -1;
    }

    if (gpu_index >= 0) {
        cuda_set_device(gpu_index);
    }

    char * video_file = "video_in.seq";
    char * weight_file = "cpuNet-4c_final.weights";
    char * cfg_file = "cpuNet-4c.cfg";
    char * output_file = "results.txt";
    char * prefix = "results_dw/image";

    demo(cfg_file, weight_file, .1f, video_file, voc_names_demo, NUM_CLASSES, 0, prefix);
}