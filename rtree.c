#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "rtree.h"

struct node *node_new(struct rtree *tr, enum kind kind) {
    struct node *node = (struct node *)tr->malloc(sizeof(struct node));
    memset(node, 0, sizeof(struct node));
    node->kind = kind;
    return node;
}

void node_free(struct rtree *tr, struct node *node) {
    if (node->kind == BRANCH) {
        for (int i = 0; i < node->count; i++) {
            node_free(tr, node->children[i]);
        }
    }
    tr->free(node);
}

void rect_expand(struct rect *rect, struct rect *other) {
    for (int i = 0; i < DIMS; i++) {
        if (other->min[i] < rect->min[i]) { rect->min[i] = other->min[i]; }
        if (other->max[i] > rect->max[i]) { rect->max[i] = other->max[i]; }
    }
}

double rect_area(struct rect *rect) {
    double area = (double)(rect->max[0]) - (double)(rect->min[0]);
    for (int i = 1; i < DIMS; i++) {
        area *= (double)(rect->max[i]) - (double)(rect->min[i]);
    }
    return area;
}

bool rect_contains(struct rect *rect, struct rect *other) {
    for (int i = 0; i < DIMS; i++) {
        if (other->min[i] < rect->min[i] || other->max[i] > rect->max[i]) {
            return false;
        }
    }
    return true;
}

bool rect_intersects(struct rect *rect, struct rect *other) {
    for (int i = 0; i < DIMS; i++) {
        if (other->min[i] > rect->max[i] || other->max[i] < rect->min[i]) {
            return false;
        }
    }
    return true;
}


bool nums_equal(NUMTYPE a, NUMTYPE b) {
    return !(a < b || a > b);
}

bool rect_onedge(struct rect *rect, struct rect *other) {
    for (int i = 0; i < DIMS; i++) {
        if (nums_equal(rect->min[i], other->min[i])) {
            return true;
        }
        if (nums_equal(rect->max[i], other->max[i])) {
            return true;
        }
    }
    return false;
}

bool rect_equals(struct rect *rect, struct rect *other) {
    for (int i = 0; i < DIMS; i++) {
        if (!nums_equal(rect->min[i], other->min[i])) {
            return false;
        }
        if (!nums_equal(rect->max[i], other->max[i])) {
            return false;
        }
    }
    return true;
}

void node_swap(struct node *node, int i, int j) {
    struct rect tmp = node->rects[i];
    node->rects[i] = node->rects[j];
    node->rects[j] = tmp;
    if (node->kind == LEAF) {
        struct item tmp = node->items[i];
        node->items[i] = node->items[j];
        node->items[j] = tmp;
    } else {
        struct node *tmp = node->children[i];
        node->children[i] = node->children[j];
        node->children[j] = tmp;
    }
}

void node_qsort(struct node *node, int s, int e, int axis, bool rev, bool max) {
    int nrects = e - s, left = 0, right = nrects - 1, pivot = nrects / 2;
    if (nrects < 2) { return; }
    node_swap(node, s + pivot, s + right);
    struct rect *rects = &node->rects[s];
    if (!rev) {
        if (!max) {
            for (int i = 0; i < nrects; i++) {
                if (rects[i].min[axis] < rects[right].min[axis]) {
                    node_swap(node, s + i, s + left);
                    left++;
                }
            }
        } else {
            for (int i = 0; i < nrects; i++) {
                if (rects[i].max[axis] < rects[right].max[axis]) {
                    node_swap(node, s + i, s + left);
                    left++;
                }
            }
        }
    } else {
        if (!max) {
            for (int i = 0; i < nrects; i++) {
                if (rects[right].min[axis] < rects[i].min[axis]) {
                    node_swap(node, s + i, s + left);
                    left++;
                }
            }
        } else {
            for (int i = 0; i < nrects; i++) {
                if (rects[right].max[axis] < rects[i].max[axis]) {
                    node_swap(node, s + i, s + left);
                    left++;
                }
            }
        }
    }
    node_swap(node, s + left, s + right);
    node_qsort(node, s, s + left, axis, rev, max);
    node_qsort(node, s + left + 1, e, axis, rev, max);
}

void node_sort(struct node *node) {
    node_qsort(node, 0, node->count, 0, false, false);
}

void node_sort_by_axis(struct node *node, int axis, bool rev, bool max) {
    node_qsort(node, 0, node->count, axis, rev, max);
}

int rect_largest_axis(struct rect *rect) {
    int axis = 0;
    double nlength = (double)rect->max[0] - (double)rect->min[0];
    for (int i = 1; i < DIMS; i++) {
        double length = (double)rect->max[i] - (double)rect->min[i];
        if (length > nlength) {
            nlength = length;
            axis = i;
        }
    }
    return axis;
}

void node_move_rect_at_index_into(struct node *from, int index, struct node *into) {
    into->rects[into->count] = from->rects[index];
    from->rects[index] = from->rects[from->count - 1];
    if (from->kind == LEAF) {
        into->items[into->count] = from->items[index];
        from->items[index] = from->items[from->count - 1];
    } else {
        into->children[into->count] = from->children[index];
        from->children[index] = from->children[from->count - 1];
    }
    from->count--;
    into->count++;
}

struct node *node_split_largest_axis_edge_snap(struct rtree *tr, struct rect *rect, struct node *left) {
    int axis = rect_largest_axis(rect);
    struct node *right = node_new(tr, left->kind);
    if (!right) return NULL;
    for (int i = 0; i < left->count; i++) {
        double min_dist = (double)left->rects[i].min[axis] - (double)rect->min[axis];
        double max_dist = (double)rect->max[axis] - (double)left->rects[i].max[axis];
        if (min_dist < max_dist) { // stay left
        } else {                   // move right
            node_move_rect_at_index_into(left, i, right);
            i--;
        }
    }
    // make sure that both left and right nodes have at least min_entries by moving items into underflowed nodes
    if (left->count < MIN_ENTRIES) { // reverse sort by min axis
        node_sort_by_axis(right, axis, true, false);
        while (left->count < 2) {
            node_move_rect_at_index_into(right, right->count-1, left);
        }
    } else if (right->count < 2) { // reverse sort by max axis
        node_sort_by_axis(left, axis, true, true);
        while (right->count < 2) {
            node_move_rect_at_index_into(left, left->count-1, right);
        }
    }
    node_sort(right);
    node_sort(left);
    return right;
}

struct node *node_split(struct rtree *tr, struct rect *r, struct node *left) {
    return node_split_largest_axis_edge_snap(tr, r, left);
}

int node_rsearch(struct node *node, NUMTYPE key) {
    for (int i = 0; i < node->count; i++) {
        if (!(node->rects[i].min[0] < key)) {
            return i;
        }
    }
    return node->count;
}

double rect_unioned_area(struct rect *rect, struct rect *other) {
    double area = (double)MAX(rect->max[0], other->max[0]) - (double)MIN(rect->min[0], other->min[0]);
    for (int i = 1; i < DIMS; i++) {
        area *= (double)MAX(rect->max[i], other->max[i]) - (double)MIN(rect->min[i], other->min[i]);
    }
    return area; // returns the area of two rects expanded
}

int node_choose_least_enlargement(struct node *node, struct rect *ir) {
    int j = -1;
    double jenlargement = 0, jarea = 0;
    for (int i = 0; i < node->count; i++) {
        // calculate the enlarged area
        double uarea = rect_unioned_area(&node->rects[i], ir);
        double area = rect_area(&node->rects[i]);
        double enlargement = uarea - area;
        if (j == -1 || enlargement < jenlargement || (!(enlargement > jenlargement) && area < jarea)) {
            j = i;
            jenlargement = enlargement;
            jarea = area;
        }
    }
    return j;
}

int node_choose_subtree(struct node *node, struct rect *ir) {
    // take a quick look for the first node that contain the rect.
    if (FAST_CHOOSER == 1) {
        int index = -1;
        double narea;
        for (int i = 0; i < node->count; i++) {
            if (rect_contains(&node->rects[i], ir)) {
                double area = rect_area(&node->rects[i]);
                if (index == -1 || area < narea) {
                    narea = area;
                    index = i;
                }
            }
        }
    } else if (FAST_CHOOSER == 2) {
        for (int i = 0; i < node->count; i++) {
            if (rect_contains(&node->rects[i], ir)) {
                return i;
            }
        }
    }
    // fallback to using the choose-least-enlargment algorithm
    return node_choose_least_enlargement(node, ir);
}

struct rect node_rect_calc(struct node *node) {
    struct rect rect = node->rects[0];
    for (int i = 1; i < node->count; i++) {
        rect_expand(&rect, &node->rects[i]);
    }
    return rect;
}

int node_order_to_right(struct node *node, int index) {
    while (index < node->count - 1 && node->rects[index + 1].min[0] < node->rects[index].min[0]) {
        node_swap(node, index + 1, index);
        index++;
    }
    return index;
}

int node_order_to_left(struct node *node, int index) {
    while (index > 0 && node->rects[index].min[0] < node->rects[index - 1].min[0]) {
        node_swap(node,index, index - 1);
        index--;
    }
    return index;
}

// performs a copy of the data from args[1] & args[2], expects a rectangle (double[] double[])
// first N values are min corner, next N values - max corner, N - num of dimensions (max coords are optional)
bool node_insert(struct rtree *tr, struct rect *nr, struct node *node, struct rect *ir, struct item item, bool *split, bool *grown) {
    *split = false;
    *grown = false;
    if (node->kind == LEAF) {
        if (node->count == MAX_ENTRIES) {
            *split = true;
            return true;
        }
        int index = node_rsearch(node, ir->min[0]);
        memmove(&node->rects[index + 1], &node->rects[index], (node->count-index) * sizeof(struct rect));
        memmove(&node->items[index + 1], &node->items[index], (node->count-index) * sizeof(struct item));
        node->rects[index] = *ir;
        node->items[index] = item;
        node->count++;
        *grown = !rect_contains(nr, ir);
        return true;
    }
    int index = node_choose_subtree(node, ir); // choose a subtree for inserting the rectangle
    if (!node_insert(tr, &node->rects[index], node->children[index], ir, item, split, grown)) {
        return false;
    }
    if (*split) {
        if (node->count == MAX_ENTRIES) {
            return true;
        }
        struct node *left = node->children[index];
        struct node *right = node_split(tr, &node->rects[index], left); // split child node
        if (!right) {
            return false;
        }
        node->rects[index] = node_rect_calc(left);
        memmove(&node->rects[index + 2], &node->rects[index + 1], (node->count - (index + 1)) * sizeof(struct rect));
        memmove(&node->children[index + 2], &node->children[index + 1], (node->count - (index + 1)) * sizeof(struct node*));
        node->rects[index + 1] = node_rect_calc(right);
        node->children[index + 1] = right;
        node->count++;
        if (node->rects[index].min[0] > node->rects[index + 1].min[0]) {
            node_swap(node, index + 1, index);
        }
        index++;
        node_order_to_right(node, index);
        return node_insert(tr, nr, node, ir, item, split, grown);
    }
    if (*grown) { // child rectangle must expand to accomadate new item
        rect_expand(&node->rects[index], ir);
        node_order_to_left(node, index);
        *grown = !rect_contains(nr, ir);
    }
    return true;
}

struct rtree *rtree_new_with_allocator(void *(*cust_malloc)(size_t), void (*cust_free)(void*)) {
    if (!cust_malloc) cust_malloc = malloc;
    if (!cust_free) cust_free = free;
    struct rtree *tr = (struct rtree *)cust_malloc(sizeof(struct rtree));
    if (!tr) { return NULL; }
    memset(tr, 0, sizeof(struct rtree));
    tr->malloc = cust_malloc;
    tr->free = cust_free;
    return tr;
}

struct rtree *rtree_new() { return rtree_new_with_allocator(NULL, NULL); }

bool rtree_insert(struct rtree *tr, const NUMTYPE *min, const NUMTYPE *max, const DATATYPE data) {
    struct rect rect;
    memcpy(&rect.min[0], min, sizeof(NUMTYPE) * DIMS);
    memcpy(&rect.max[0], max ? max : min, sizeof(NUMTYPE) * DIMS);
    struct item item;
    memcpy(&item.data, &data, sizeof(DATATYPE));
    if (!tr->root) {
        struct node *new_root = node_new(tr, LEAF);
        if (!new_root) return false;
        tr->root = new_root;
        tr->rect = rect;
    }
    bool split = false, grown = false;
    if (!node_insert(tr, &tr->rect, tr->root, &rect, item, &split, &grown)) { return false; }
    if (split) {
        struct node *new_root = node_new(tr, BRANCH);
        if (!new_root) return false;
        struct node *left = tr->root;
        struct node *right = node_split(tr, &tr->rect, left);
        tr->root = new_root;
        tr->root->rects[0] = node_rect_calc(left);
        tr->root->rects[1] = node_rect_calc(right);
        tr->root->children[0] = left;
        tr->root->children[1] = right;
        tr->root->count = 2;
        tr->height++;
        node_sort(tr->root);
        return rtree_insert(tr, min, max, data);
    }
    if (grown) {
        rect_expand(&tr->rect, &rect);
        node_sort(tr->root);
    }
    tr->count++;
    return true;
}

void rtree_free(struct rtree *tr) {
    if (tr->root) { node_free(tr, tr->root); }
    tr->free(tr);
}

bool node_search(struct node *node, struct rect *rect, bool (*iter)(const NUMTYPE *min, const NUMTYPE *max, const DATATYPE data, void *udata), void *udata) {
    if (node->kind == LEAF) {
        for (int i = 0; i < node->count; i++) {
            if (rect_intersects(&node->rects[i], rect)) {
                if (!iter(node->rects[i].min, node->rects[i].max, node->items[i].data, udata)) {
                    return false;
                }
            }
        }
        return true;
    }
    for (int i = 0; i < node->count; i++) {
        if (rect_intersects(&node->rects[i], rect)) {
            if (!node_search(node->children[i], rect, iter, udata)) {
                return false;
            }
        }
    }
    return true;
}

void rtree_search(struct rtree *tr, const NUMTYPE *min, const NUMTYPE *max, bool (*iter)(const NUMTYPE *min, const NUMTYPE *max, const DATATYPE data, void *udata), void *udata) {
    struct rect rect;
    memcpy(&rect.min[0], min, sizeof(NUMTYPE) * DIMS);
    memcpy(&rect.max[0], max ? max : min, sizeof(NUMTYPE) * DIMS);
    if (tr->root && rect_intersects(&tr->rect, &rect)) {
        node_search(tr->root, &rect, iter, udata);
    }
}

size_t rtree_count(struct rtree *tr) { return tr->count; }

void node_delete(struct rtree *tr, struct rect *nr, struct node *node, struct rect *ir, struct item item, bool *removed, bool *shrunk, int (*compare)(const DATATYPE a, const DATATYPE b, void *udata), void *udata) {
    *removed = false;
    *shrunk = false;
    if (node->kind == LEAF) {
        for (int i = 0; i < node->count; i++) {
            if (!rect_contains(ir, &node->rects[i])) {
                continue;
            }
            int cmp = compare ?
                compare(node->items[i].data, item.data, udata) :
                memcmp(&node->items[i].data, &item.data, sizeof(DATATYPE));
            if (cmp != 0) {
                continue;
            }
            // found the target item to delete
            memmove(&node->rects[i], &node->rects[i + 1], (node->count - (i + 1)) * sizeof(struct rect));
            memmove(&node->items[i], &node->items[i + 1], (node->count - (i + 1)) * sizeof(struct item));
            node->count--;
            if (rect_onedge(ir, nr)) {      // item was on the edge of node rect
                *nr = node_rect_calc(node); // recalculation of node rect
                *shrunk = true;             // notify the caller that rect is shrunk
            }
            *removed = true;
            return;
        }
        return;
    }
    for (int i = 0; i < node->count; i++) {
        if (!rect_contains(&node->rects[i], ir)) {
            continue;
        }
        struct rect crect = node->rects[i];
        node_delete(tr, &node->rects[i], node->children[i], ir, item, removed, shrunk, compare, udata);
        if (!*removed) {
            continue;
        }
        if (node->children[i]->count == 0) { // underflow
            node_free(tr, node->children[i]);
            memmove(&node->rects[i], &node->rects[i + 1], (node->count - (i + 1)) * sizeof(struct rect));
            memmove(&node->children[i], &node->children[i + 1], (node->count - (i + 1)) * sizeof(struct node *));
            node->count--;
            *nr = node_rect_calc(node);
            *shrunk = true;
            return;
        }
        if (*shrunk) {
            *shrunk = !rect_equals(&node->rects[i], &crect);
            if (*shrunk) {
                *nr = node_rect_calc(node);
            }
            node_order_to_right(node, i);
        }
        return;
    }
    return;
}

// search the tree for an item contained within provided rect, perform a binary comparison of its data to provided, first item found is deleted
void rtree_delete(struct rtree *tr, const NUMTYPE *min, const NUMTYPE *max, const DATATYPE data) {
    struct rect rect;
    memcpy(&rect.min[0], min, sizeof(NUMTYPE) * DIMS);
    memcpy(&rect.max[0], max ? max : min, sizeof(NUMTYPE) * DIMS);
    struct item item;
    memcpy(&item.data, &data, sizeof(DATATYPE));
    if (!tr->root) { return; }
    bool removed = false, shrunk = false;
    node_delete(tr, &tr->rect, tr->root, &rect, item, &removed, &shrunk, NULL, NULL);
    if (!removed) {
        return;
    }
    tr->count--;
    if (tr->count == 0) {
        node_free(tr, tr->root);
        tr->root = NULL;
        memset(&tr->rect, 0, sizeof(struct rect));
    } else {
        while (tr->root->kind == BRANCH && tr->root->count == 1) {
            struct node *prev = tr->root;
            tr->root = tr->root->children[0];
            prev->count = 0;
            node_free(tr, prev);
        }
        if (shrunk) {
            tr->rect = node_rect_calc(tr->root);
        }
    }
}