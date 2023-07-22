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




    int* p0 = mm_malloc(4096);
    int* p1 = mm_malloc(4096);
    int* p2 = mm_malloc(4096);
    int* p3 = mm_malloc(4096);
    int* p4 = mm_malloc(4096);
    int* p5 = mm_malloc(4096);
    int* p6 = mm_malloc(4096);
    mm_free(p0);
    mm_free(p1);
    mm_free(p2);
    mm_free(p3);
    mm_free(p4);
    mm_free(p5);
    mm_free(p6);
    p0 = mm_malloc(4096);
    p1 = mm_malloc(4096);
    p2 = mm_malloc(4096);
    p3 = mm_malloc(4096);
    p4 = mm_malloc(4096);
    p5 = mm_malloc(4096);
    p6 = mm_malloc(4096);
    mm_free(p0);
    mm_free(p1);
    mm_free(p2);
    mm_free(p3);
    mm_free(p4);
    mm_free(p5);
    mm_free(p6);
    return 0;
}