#ifndef DARKNET_DARKNET_WRAPPER_H
#define DARKNET_DARKNET_WRAPPER_H

void darknet_wrapper_init(
        const char *cfg_fname,
        const char *weights_fname,
        const char *video_in_fname,
        const char *out_bboxes_fname,
        const char *out_img_dir);

#endif //DARKNET_DARKNET_WRAPPER_H
