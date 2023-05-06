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
struct city spb = { .name = "Saint-Petersburg", .latitude = 59.9500, .longitude = 30.3167 };
struct city pra = { .name = "Prague", .latitude = 50.088, .longitude = 14.420 };
struct city alm = { .name = "Almaty", .latitude = 43.2775, .longitude = 76.8958 };
struct city bai = { .name = "Buenos Aires", .latitude = -34.5997, .longitude = -58.3819 };
struct city rio = { .name = "Rio de Janeiro", .latitude = -22.9111, .longitude = -43.2056 };
struct city tok = { .name = "Tokyo", .latitude = 35.6897, .longitude = 139.6922 };
struct city szh = { .name = "Shenzhen", .latitude = 22.5350, .longitude = 114.0540 };

bool city_iter(const double *min, const double *max, const void *item, void *udata) {
    const struct city *city = item;
    printf("%s\n", city->name);
    return true;
}

int main() {
    struct rtree *tr = rtree_new();

    // load cities into the rtree, expecting a rectangle (2 arrs of doubles) on input
    // insert() also performs a copy of the pointed data at args[1] & [2]
    // first N vals as min corner and the next N vals as max corner (max are optional)
    // default number of dimensions - 2
    rtree_insert(tr, (double[2]){nsk.longitude, nsk.latitude}, NULL, &nsk);
    rtree_insert(tr, (double[2]){spb.longitude, spb.latitude}, NULL, &spb);
    rtree_insert(tr, (double[2]){pra.longitude, pra.latitude}, NULL, &pra);
    rtree_insert(tr, (double[2]){alm.longitude, alm.latitude}, NULL, &alm);
    rtree_insert(tr, (double[2]){bai.longitude, bai.latitude}, NULL, &bai);
    rtree_insert(tr, (double[2]){rio.longitude, rio.latitude}, NULL, &rio);
    rtree_insert(tr, (double[2]){tok.longitude, tok.latitude}, NULL, &tok);
    rtree_insert(tr, (double[2]){szh.longitude, szh.latitude}, NULL, &szh);
    
    printf("\nNorthwestern cities:\n");
    rtree_search(tr, (double[2]){-180, 0}, (double[2]){0, 90}, city_iter, NULL);
    printf("\nNortheastern cities:\n");
    rtree_search(tr, (double[2]){0, 0}, (double[2]){180, 90}, city_iter, NULL);

    rtree_delete(tr, (double[2]){nsk.longitude, nsk.latitude}, NULL, &nsk);
    printf("\nNortheastern cities:\n");
    rtree_search(tr, (double[2]){0, 0}, (double[2]){180, 90}, city_iter, NULL);
    rtree_free(tr);
}