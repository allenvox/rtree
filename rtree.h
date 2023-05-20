#pragma once

#include <stdlib.h>
#include <stdbool.h>

#define DATATYPE void * 
#define NUMTYPE double
#define DIMS 2
#define MAX_ENTRIES 64
#define MIN_ENTRIES_PERCENTAGE 10
#define FAST_CHOOSER 2  // 0 - off , 1 - fast, 2 - faster
#define panic(_msg_) { \
    fprintf(stderr, "panic: %s (%s:%d)\n", (_msg_), __FILE__, __LINE__); \
    exit(1); \
}
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN_ENTRIES ((MAX_ENTRIES) * (MIN_ENTRIES_PERCENTAGE) / 100 + 1)

enum kind {
    LEAF = 1,
    BRANCH = 2,
};

struct rect {
    NUMTYPE min[DIMS];
    NUMTYPE max[DIMS];
};

struct item {
    DATATYPE data;
};

struct node {
    enum kind kind;     // LEAF or BRANCH
    int count;          // number of rects
    struct rect rects[MAX_ENTRIES];
    union { struct node *children[MAX_ENTRIES]; struct item items[MAX_ENTRIES]; };
};

struct rtree {
    size_t count;
    int height;
    struct rect rect;
    struct node *root; 
    void *(*malloc)(size_t);
    void (*free)(void *);
};

struct rtree *rtree_new();
bool rtree_insert(struct rtree *tr, const double *min, const double *max, const void *data);
void rtree_free(struct rtree *tr);
void rtree_search(struct rtree *tr, const double *min, const double *max, bool (*iter)(const double *min, const double *max, const void *data, void *udata), void *udata);
size_t rtree_count(struct rtree *tr);
void rtree_delete(struct rtree *tr, const double *min, const double *max, const void *data);