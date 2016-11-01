import shapely.ops
import shapely.geometry
import operator
import sys

if len(sys.argv) != 4:
    print("usage {}: <predicted boxes> <annotated boxes> <thresh>".format(sys.argv[0]))
    sys.exit()

RESULTS_FILENAME = sys.argv[1]
LABELS_FILENAME = sys.argv[2]
THRESH = sys.argv[3]

def get_boxes_data(filename, with_frame_count=False):
    boxes_data = {}
    frame_count = 0
    with open(filename, 'r') as file:
        if with_frame_count:
            frame_count = int(file.readline())
        for line in file:
            frame, left, top, right, bottom = [int(x) for x in line.split()]
            boxes_data[frame] = boxes_data.get(frame, []) + [(left, top, right, bottom)]
    return (boxes_data, frame_count) if with_frame_count else boxes_data


def get_frame_acc_definitions(predicted_boxes, true_boxes):
    if not len(predicted_boxes) and not len(true_boxes):
        return 0, 0, 0, 1
    elif not len(true_boxes):
        return 0, len(predicted_boxes), 0, 0
    elif not len(predicted_boxes):
        return 0, 0, len(true_boxes), 0

    p_boxes, t_boxes = [[shapely.geometry.box(box[0], box[1], box[2], box[3]) for box in boxes_list]
                        for boxes_list in [predicted_boxes, true_boxes]]

    true_boxes_maxs = [sorted(p_boxes, key=(lambda b: lambda p_box: p_box.intersection(b).area)(t_box), reverse=True)
                       for t_box in t_boxes]

    predicted_boxes_maxs = []
    for p_box in p_boxes:
        p_box_idxs = [maxs.index(p_box) for maxs in true_boxes_maxs]
        p_box_min_idxs = [idx for idx, item in enumerate(p_box_idxs) if item == min(p_box_idxs)]

        predicted_boxes_maxs.append(
            max([t_boxes[i] for i in p_box_min_idxs],
                key=(lambda b: lambda t_box: t_box.intersection(b).area)(p_box)))

    TP = 0
    FP = 0
    for idx, t_box in enumerate(predicted_boxes_maxs):
        if t_box.intersection(p_boxes[idx]).area / shapely.ops.unary_union([t_box, p_boxes[idx]]).area > THRESH:
            TP += 1
        else:
            FP += 1

    return TP, FP, len(t_boxes) - TP, 0


predicted_data = get_boxes_data(RESULTS_FILENAME)
true_data, frame_count = get_boxes_data(LABELS_FILENAME, True)

frames = set(predicted_data.keys() + true_data.keys())

TP, FP, FN, TN = (0, 0, 0, 0)

for i in frames:
    acc_definitions = get_frame_acc_definitions(predicted_data.get(i, []), true_data.get(i, []))
    TP, FP, FN, TN = map(operator.add, acc_definitions, (TP, FP, FN, TN))

TN += frame_count - len(frames)

ACC = float(TP + TN) / float(TP + FP + FN + TN)

print('TP = {}, FP = {}, FN = {}, TN = {}\nACC = {}'.format(TP, FP, FN, TN, ACC))
