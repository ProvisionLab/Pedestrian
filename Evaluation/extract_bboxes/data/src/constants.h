#ifndef CONSTANTS_H
#define CONSTANTS_H

// Applied only for YOLO model!
//#define CLASS_NAMES {"person"}
#define CLASS_NAMES {"person", "people", "person?", "person-fa"}
//#define NUM_CLASSES 1
#define NUM_CLASSES 4
#define TRAIN_IMAGES_PATHS_FILE "data/caltech_pedestrians/train.txt"
#define TEST_IMAGES_PATHS_FILE  "data/caltech_pedestrians/test.txt"
#define BACKUP_DIR "backup/"

#endif //CONSTANTS_H
