#pragma once

#include <stdlib.h>
#include <stdbool.h>

struct rtree *rtree_new();

struct rtree *rtree_new_with_allocator(void *(*malloc)(size_t), void (*free)(void*));

bool rtree_insert(struct rtree *tr, const double *min, const double *max, const void *data);

void rtree_free(struct rtree *tr);

void rtree_search(struct rtree *tr, const double *min, const double *max,
    bool (*iter)(const double *min, const double *max, const void *data, void *udata), 
    void *udata);

size_t rtree_count(struct rtree *tr);

void rtree_delete(struct rtree *tr, const double *min, const double *max, const void *data);

void rtree_delete_with_comparator(struct rtree *tr, const double *min, 
    const double *max, const void *data,
    int (*compare)(const void *a, const void *b, void *udata),
    void *udata);