import shapely.ops
import shapely.geometry
import sys

if len(sys.argv) != 4:
    print("usage {}: <processed bboxes> <raw bboxes> <thresh>".format(sys.argv[0]))
    sys.exit()

RESULTS_FILENAME = sys.argv[1]
LABELS_FILENAME = sys.argv[2]
THRESH = sys.argv[3]


def get_bboxes_data(filename):
    bboxes_data = {}
    with open(filename, 'r') as file:
        for line in file:
            frame, left, top, right, bottom = [int(x) for x in line.split()]
            bboxes_data.setdefault(frame, [])
            bboxes_data[frame].append((left, top, right, bottom))
    return bboxes_data


def get_ratio(rect_bb_1, rect_bb_2):
    boxes = [shapely.geometry.box(rect_bb[0], rect_bb[1], rect_bb[2], rect_bb[3]) for rect_bb in [rect_bb_1, rect_bb_2]]
    intersection_area = boxes[0].intersection(boxes[1]).area
    return boxes[0].intersection(boxes[1]).area / shapely.ops.unary_union(boxes).area \
        if intersection_area else 0.0


def calculate_accuracy(results_rect_list, labels_rect_list):
    if bool(len(results_rect_list)) - bool(len(labels_rect_list)):
        return 0.0

    if not len(results_rect_list) and not len(labels_rect_list):
        return 1.0

    true_positives = 0
    false_positives = 0

    last_labels_rect_list = labels_rect_list[:]

    for result_rect in results_rect_list:
        ratio_list = [get_ratio(result_rect, label_rect) for label_rect in last_labels_rect_list]
        max_ratio = max(ratio_list) if len(ratio_list) else 0.0
        if max_ratio > float(THRESH):
            last_labels_rect_list.remove(last_labels_rect_list[ratio_list.index(max_ratio)])
            true_positives += 1
        else:
            false_positives += 1

    true_negative = max(0, len(labels_rect_list) - false_positives)
    return float(true_positives + true_negative) / (len(labels_rect_list) + len(results_rect_list))


results_bboxes_data = get_bboxes_data(RESULTS_FILENAME)
labels_bboxes_data = get_bboxes_data(LABELS_FILENAME)

accuracies = [calculate_accuracy(results_bboxes_data.get(frame, []), labels_bboxes_data.get(frame, []))
            for frame in set(results_bboxes_data.keys() + labels_bboxes_data.keys())]

avg_accuracy = sum(accuracies) / float(len(accuracies))

print('Accuracy: {}%'.format(avg_accuracy * 100.))
