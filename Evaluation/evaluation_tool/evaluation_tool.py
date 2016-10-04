import shapely.ops
import shapely.geometry

RESULTS_FILENAME = 'bboxes.txt'
LABELS_FILENAME = 'bboxes_labeled.txt'


def get_bboxes_data(filename):
    bboxes_data = {}
    with open(filename, 'r') as file:
        for line in file:
            frame, left, top, right, bottom = [int(x) for x in line.split()]
            bboxes_data.setdefault(frame, [])
            bboxes_data[frame].append((left, top, right, bottom))
    return bboxes_data;


def get_union(rect_list):
    return shapely.ops.unary_union([shapely.geometry.box(bb[0], bb[1], bb[2], bb[3]) for bb in rect_list])


def calculate_overlap_ratio(results_rect_list, labels_rect_list):
    if (not len(results_rect_list) and len(labels_rect_list)) or (len(results_rect_list) and not len(labels_rect_list)):
        return 0.0

    SI = get_union(results_rect_list).intersection(get_union(labels_rect_list)).area
    S = get_union(results_rect_list + labels_rect_list).area
    return SI / S


results_bboxes_data = get_bboxes_data(RESULTS_FILENAME)
labels_bboxes_data = get_bboxes_data(LABELS_FILENAME)

overlaps = [calculate_overlap_ratio(results_bboxes_data.get(frame, []), labels_bboxes_data.get(frame, []))
            for frame in set(results_bboxes_data.keys() + labels_bboxes_data.keys())]
avg_overlap_ratio = sum(overlaps) / float(len(overlaps))

print('Accuracy: {}%'.format(avg_overlap_ratio * 100.))
