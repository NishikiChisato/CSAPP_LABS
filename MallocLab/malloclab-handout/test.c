#include <stdio.h>
#include "mm.h"
#include "memlib.h"

int main()
{
    mem_init();
    mm_init();
    int* p1 = mm_malloc(2040);
    mm_free(p1);
    return 0;
}