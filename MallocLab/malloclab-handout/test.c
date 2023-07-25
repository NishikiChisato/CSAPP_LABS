#include <stdio.h>
#include "mm.h"
#include "memlib.h"

int main()
{
    mem_init();
    mm_init();

    // int* p0 = mm_malloc(512);
    // int* p1 = mm_malloc(128);
    // p0 = mm_realloc(p0, 640);    
    // int* p2 = mm_malloc(128);
    // mm_free(p1);
    // p0 = mm_realloc(p0, 768);
    // int* p3 = mm_malloc(128);

    int* p0 = mm_malloc(4075);
    int* p1 = mm_malloc(4072);
    int* p2 = mm_malloc(4062);
    int* p3 = mm_malloc(4059);
    int* p4 = mm_malloc(4056);
    int* p5 = mm_malloc(4075);
    int* p6 = mm_malloc(4035);
    mm_free(p0);
    mm_free(p1);
    mm_free(p2);
    mm_free(p3);
    mm_free(p4);
    mm_free(p5);
    mm_free(p6);
    p0 = mm_malloc(4075);
    p1 = mm_malloc(4072);
    p2 = mm_malloc(4062);
    p3 = mm_malloc(4059);
    p4 = mm_malloc(4056);
    p5 = mm_malloc(4075);
    p6 = mm_malloc(4035);
    mm_free(p0);
    mm_free(p1);
    mm_free(p2);
    mm_free(p3);
    mm_free(p4);
    mm_free(p5);
    mm_free(p6);


    return 0;
}