import cv2
import scipy.io as sio
import copy
import os
import glob
import pathlib2

PATH_TO_DATA = "../data"
PATH_TO_EXTRACT = "../extraction_data"
IMG_SIZE = (640, 480)


def convert_bb(x, y, w, h):
    dw = 1./IMG_SIZE[0]
    dh = 1./IMG_SIZE[1]

    x = (2.0 * x + w) / 2.0
    y = (2.0 * y + h) / 2.0
    x *= dw
    w *= dw
    y *= dh
    h *= dh
    return x, y, w, h


def parse_annotations(filename):
    vbb = sio.loadmat(filename)
    objLists = vbb['A'][0][0][1][0][0:]
    objLbl = [str(v[0]) for v in vbb['A'][0][0][4][0]]

    def extract_params(obj):
        return [{'label_id': int(label_id[0][0]) - 1,  # MATLAB is 1-origin
                 'label': objLbl[int(label_id[0][0]) - 1],
                 'pos': [round(x) for x in pos[0].tolist()],
                 'posv': [round(x) for x in posv[0].tolist()]}
                for label_id, pos, posv in
                zip(obj['id'][0], obj['pos'][0], obj['posv'][0])]

    return [extract_params(obj) if obj.size else [] for obj in objLists]


def parse_seq(filename, frame_numbers=[]):
    cap = cv2.VideoCapture(filename)

    if cap.isOpened():
        images = []
        grab_counter = 0
        for frame_number in frame_numbers:
            while grab_counter < frame_number - 1:
                if not cap.grab():
                    break
                grab_counter += 1

            ret, frame = cap.read()
            grab_counter += 1
            if ret:
                images.append(frame)

        return images

    return []


def images_with_bbs(image_list, annotations):
    assert (len(image_list) == len(annotations))
    image_list_with_bb = copy.deepcopy(image_list)
    for img, annotation in zip(image_list_with_bb, annotations):
        for bb_info in annotation:
            text = '{}[{}]'.format(bb_info['label'], bb_info['label_id'])
            pos = bb_info['pos']
            cv2.rectangle(img,
                          tuple(pos[:2]),
                          (pos[0] + pos[2], pos[1] + pos[3]),
                          (0, 255, 0),
                          2)
    return image_list_with_bb


annotation_set_paths = sorted(glob.glob(os.path.join(PATH_TO_DATA, 'annotations/set*')))
seq_set_paths = sorted(glob.glob(os.path.join(PATH_TO_DATA, 'videos/set*')))

for annotation_set_dir, seq_set_dir in zip(annotation_set_paths, seq_set_paths):
    annotation_files = sorted(glob.glob('{}/*.vbb'.format(annotation_set_dir)))
    seq_files = sorted(glob.glob('{}/*.seq'.format(seq_set_dir)))

    out_dir = os.path.join(PATH_TO_EXTRACT, os.path.basename(annotation_set_dir))
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    images_dir = os.path.join(out_dir, 'images')
    if not os.path.exists(images_dir):
        os.makedirs(images_dir)

    labels_dir = os.path.join(out_dir, 'labels')
    if not os.path.exists(labels_dir):
        os.makedirs(labels_dir)

    with open(os.path.join(PATH_TO_EXTRACT, 'all.txt'), 'a') as path_file:
        for ann, seq in zip(annotation_files, seq_files):
            labels_seq_dir = os.path.join(labels_dir, os.path.basename(os.path.splitext(ann)[0]))
            if not os.path.exists(labels_seq_dir):
                os.makedirs(labels_seq_dir)

            nonempty_frame_numbers = []
            for i, ann_list in enumerate(parse_annotations(ann)):
                if len(ann_list):
                    nonempty_frame_numbers.append(i)
                    with open('{}/{}.txt'.format(labels_seq_dir, len(nonempty_frame_numbers) - 1), 'w') as label_file:
                        for bb_info in ann_list:
                            x, y, w, h = convert_bb(*bb_info['pos'])
                            label_file.write('0 {} {} {} {}\n'.format(x, y, w, h))

            images_seq_dir = os.path.join(images_dir, os.path.basename(os.path.splitext(seq)[0]))
            if not os.path.exists(images_seq_dir):
                os.makedirs(images_seq_dir)

            for i, img in enumerate(parse_seq(seq, nonempty_frame_numbers)):
                image_path = '{}/{}.png'.format(images_seq_dir, i)
                cv2.imwrite(image_path, img)
                path_file.write('{}\n'.format(pathlib2.Path(*pathlib2.Path(image_path).parts[2:])))

            print('Processed {} and {}'.format(ann, seq))

print('Extraction complete')
