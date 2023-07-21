#include <stdio.h>
#include "mm.h"
#include "memlib.h"

int main()
{
    mem_init();
    mm_init();
    int* p0 = mm_malloc(4010);
    int* p1 = mm_malloc(4010);
    int* p2 = mm_malloc(4010);
    int* p3 = mm_malloc(4072);
    int* p4 = mm_malloc(4000);
    int* p5 = mm_malloc(4032);
    int* p6 = mm_malloc(4030);
    mm_free(p0);
    mm_free(p1);
    mm_free(p2);
    mm_free(p3);
    mm_free(p4);
    mm_free(p5);
    mm_free(p6);
    p0 = mm_malloc(4010);
    p1 = mm_malloc(4010);
    p2 = mm_malloc(4010);
    p3 = mm_malloc(4072);
    p4 = mm_malloc(4000);
    p5 = mm_malloc(4032);
    p6 = mm_malloc(4030);
    mm_free(p0);
    mm_free(p1);
    mm_free(p2);
    mm_free(p3);
    mm_free(p4);
    mm_free(p5);
    mm_free(p6);
    return 0;
}