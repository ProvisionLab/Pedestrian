#include "darknet_wrapper.h"
#include "network.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "utils.h"
#include "parser.h"
#include <sys/stat.h>
#include <errno.h>
#include <opencv2/highgui/highgui_c.h>

image get_image_from_stream(CvCapture *cap);

#define FRAMES 3

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
static float demo_thresh = 0;

static float *predictions[FRAMES];
static int demo_index = 0;
static image images[FRAMES];
static float *avg;

int fetch() {
    in = get_image_from_stream(cap);
    if (!in.data) {
        return 0;
    }
    in_s = resize_image(in, net.w, net.h);
    return 1;
}

void detect() {
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

    images[demo_index] = det;
    det = images[(demo_index + FRAMES/2 + 1)%FRAMES];
    demo_index = (demo_index + 1)%FRAMES;

    int num_pos = l.w*l.h*l.n;
    //allocate det_pos
    int a1 = sizeof(*det_pos);
    int a2 = sizeof(image_pos);

    if (det_pos)
        free(det_pos);

    det_pos = (image_pos *)calloc((size_t)num_pos, sizeof(*det_pos));
    int a3 = 200;

    size_det_pos = 0;
    int demo_classes = l.classes;
    get_detection_bboxes(det, num_pos, demo_thresh, boxes, probs, demo_classes, det_pos, &size_det_pos);
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
}

void darknet_wrapper_init(
        const char *cfg_fname,
        const char *weights_fname,
        const char *video_in_fname,
        const char *out_bboxes_fname,
        const char *out_img_dir)
{
    if (gpu_index >= 0) {
        cuda_set_device(gpu_index);
    }

    demo_thresh = .1f;
    int frame_skip = 0;

    int delay = frame_skip;

    net = parse_network_cfg((char *)cfg_fname);
    load_weights(&net, (char *)weights_fname);
    set_batch_network(&net, 1);

    layer l = net.layers[net.n-1];

    srand(2222222);

    cap = cvCaptureFromFile(video_in_fname);
    if(!cap)
        error("Couldn't load video\n");

    avg = (float *)calloc((size_t)l.outputs, sizeof(float));
    for(int j = 0; j < FRAMES; ++j)
        predictions[j] = (float *) calloc((size_t)l.outputs, sizeof(float));
    for(int j = 0; j < FRAMES; ++j)
        images[j] = make_image(1,1,3);

    boxes = (box *)calloc((size_t)(l.w*l.h*l.n), sizeof(box));
    probs = (float **)calloc((size_t)(l.w*l.h*l.n), sizeof(float *));
    for(int j = 0; j < l.w*l.h*l.n; ++j)
        probs[j] = (float *)calloc((size_t)l.classes, sizeof(float *));

    fetch();
    disp = det;
    det = in;
    det_s = in_s;

    for(int j = 0; j < FRAMES/2 + 1; ++j) {
        fetch();
        detect();
        disp = det;
        det = in;
        det_s = in_s;
    }

    int count = 0;

    remove(out_bboxes_fname);
    FILE *file_bboxes = fopen((char *)out_bboxes_fname, "a");
    if (!file_bboxes) {
        printf("Error opening file!\n");
    }

    if (mkdir(out_img_dir, 0700) == -1 && errno == EEXIST) {
        char buff[256];
        sprintf(buff, "exec rm -rf %s/*", out_img_dir);
        system(buff);
    }

    while (1) {
        ++count;

        if (!fetch())
            break;

        detect();

        char buff[256];
        sprintf(buff, "%s/%d", out_img_dir, count);
        save_image(disp, buff);

        for (int p = 0; p < size_det_pos; ++p) {
            fprintf(file_bboxes, "%d %d %d %d %d\n", count, det_pos[p].left, det_pos[p].top, det_pos[p].right, det_pos[p].bottom);
        }

        if (delay == 0) {
            free_image(disp);
            disp  = det;
        }
        det   = in;
        det_s = in_s;

        --delay;
        if(delay < 0){
            delay = frame_skip;
        }
    }

    fclose(file_bboxes);
}