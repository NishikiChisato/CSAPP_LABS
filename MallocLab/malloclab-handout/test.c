#include <stdio.h>
#include "mm.h"
#include "memlib.h"

int main()
{
    mem_init();
    mm_init();
    int* p0 = mm_malloc(2040);
    int* p1 = mm_malloc(2040);
    mm_free(p1);
    int* p2 = mm_malloc(48);
    mm_free(p0);
    mm_free(p2);
    return 0;
}