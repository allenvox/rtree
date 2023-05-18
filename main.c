#include <stdio.h>
#include <string.h>
#include <math.h>
#include "rtree.h"

struct city {
    char *name;
    double latitude;
    double longitude;
};

struct city nsk = { .name = "Novosibirsk", .latitude = 55.0333, .longitude = 82.9167 };
struct city bai = { .name = "Buenos Aires", .latitude = -34.5997, .longitude = -58.3819 };
struct city rio = { .name = "Rio de Janeiro", .latitude = -22.9111, .longitude = -43.2056 };
struct city tok = { .name = "Tokyo", .latitude = 35.6897, .longitude = 139.6922 };
struct city tor = { .name = "Toronto", .latitude = 43.6532, .longitude = -79.3832 };
struct city syd = { .name = "Sydney", .latitude = -33.8688, .longitude = 151.2093 };

bool city_iter(const double *min, const double *max, const void *item, void *udata) {
    const struct city *city = item;
    printf("%s\n", city->name);
    return true;
}

int main() {
    struct rtree *tr = rtree_new();
    rtree_insert(tr, (double[2]){nsk.longitude, nsk.latitude}, NULL, &nsk);
    rtree_insert(tr, (double[2]){tor.longitude, tor.latitude}, NULL, &tor);
    rtree_insert(tr, (double[2]){bai.longitude, bai.latitude}, NULL, &bai);
    rtree_insert(tr, (double[2]){rio.longitude, rio.latitude}, NULL, &rio);
    rtree_insert(tr, (double[2]){tok.longitude, tok.latitude}, NULL, &tok);
    rtree_insert(tr, (double[2]){syd.longitude, syd.latitude}, NULL, &syd);
    
    printf("\nNorthwestern cities:\n");
    rtree_search(tr, (double[2]){-180, 0}, (double[2]){0, 90}, city_iter, NULL);
    printf("\nNortheastern cities:\n");
    rtree_search(tr, (double[2]){0, 0}, (double[2]){180, 90}, city_iter, NULL);
    printf("\nSouthwestern cities:\n");
    rtree_search(tr, (double[2]){-180, -90}, (double[2]){0, 0}, city_iter, NULL);
    printf("\nSoutheastern cities:\n");
    rtree_search(tr, (double[2]){0, -90}, (double[2]){180, 0}, city_iter, NULL);

    rtree_delete(tr, (double[2]){nsk.longitude, nsk.latitude}, NULL, &nsk);
    printf("\nNortheastern cities after element deletion:\n");
    rtree_search(tr, (double[2]){0, 0}, (double[2]){180, 90}, city_iter, NULL);
    rtree_free(tr);
}