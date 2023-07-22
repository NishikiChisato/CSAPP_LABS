# malloclab

- [malloclab](#malloclab)
  - [准备](#准备)
  - [实验开始](#实验开始)
    - [基本思路](#基本思路)
    - [测试](#测试)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

注：本实验无测试文件，我的 `trace` 测试文件是从 [这里](https://github.com/pgoodman/csc369/tree/master/malloclab/traces) 下载的

## 实验开始

> 注：本实验我的代码最终仅仅得分 `73/100`，由于在此实验上耗费的时间实在太多（最开始思路想错了），因此我决定不再修改代码
>
> 文件中 `test.c` 为测试文件，编译时用 `gcc -g -m32 -o debug test.c mm.c memlib.c` 进行编译

### 基本思路

设每次调用 `mm_malloc` 请求 $n$ 个字节，我们以 $2$ 的幂次向上取整进行返回，换句话说，我们每次返回大于等于 $n$ 的最小的 $2$ 的幂

我们可以用以下函数快速求出 $2$ 的幂次

```c
size_t roundUp2Pow(size_t num)
{
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;
    return num;
}
```

因此，我们可以维护一个 `CLASS_SIZE` 大小的数组，数组的每个元素都指向一个特定大小的链表，也就是说每个链表都是一个等价类 `size class`，大小为 $2$ 的幂次

因此，**我们可以 $O(1)$ 的时间返回和申请内存，但相应地我们平均会浪费 $2^{n}$ 个字节的内存**

完整代码如下，先放在这里，有时间我会回来重做这个实验

```c
/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Atom",
    /* First member's full name */
    "Nishiki Chisato",
    /* First member's email address */
    "NishikiChisato@outlook.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//single word size
//header and footer both is WSIZE
#define WSIZE           4
//double word size
#define DSIZE           8
#define CHUNKSIZE       (1 << 12)
#define MAX(x, y)       ((x) > (y) ? (x) : (y))
#define PACK(size, alloc)   ((size) |(alloc))

//every word has 4 bytes
//read a word from p
#define GET(p)          (*(unsigned int*)(p))
//write a word to p
#define PUT(p, val)     (*(unsigned int*)(p) = (val))

//get block size(include header and footer) from p
//in other word, block size = payload size + header size + foot size = payload size + 8
#define GET_SIZE(p)     (GET(p) & ~0x7)
//determine whether p has been allocated
#define GET_ALLOC(p)    (GET(p) & 0x1)

//get header and footer
//bp is block ptr, block is payload area
#define HDRP(bp)        ((char*)(bp) - WSIZE)
#define FTRP(bp)        ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

//get next block ptr and prev block ptr
#define NEXT_BLKP(bp)   ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp)   ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))

#define GET_PRED(bp)    ((unsigned int*)(bp))
#define GET_SUCC(bp)    ((unsigned int*)(bp + WSIZE))

//free list index
#define GET_FREE_LIST(num)  (free_list + WSIZE + ((num) * WSIZE))

//the size of size class
#define CLASS_SIZE  30
//free list entries' size
//min val is 3
#define ENTRY_SIZE  3

static char* free_list;

static void* extend_heap(size_t words);


void FreeBlock(void* bp)
{
    int i = 0;
    for(void* cur = bp; GET_SIZE(HDRP(cur)) > 0; cur = NEXT_BLKP(cur))
    {
        printf("i = %d, begin: %p, end: %p\n",i++ , cur, FTRP(cur));
    }
}

void BlockPtr(void* bp)
{
    int i = 0;
    for(void* cur = bp; GET_SIZE(HDRP(cur)) > 0; cur = NEXT_BLKP(cur))
    {
        printf("i = %d, begin: %p, end: %p, pred: %p, succ: %p, size: %d\n",i++ , cur, FTRP(cur), 
        GET(GET_PRED(cur)), GET(GET_SUCC(cur)), GET_SIZE(HDRP(cur)));
    }
}

void Linklist(void* bp)
{
    int i = 0;
    for(void* cur = bp; GET_SIZE(HDRP(cur)) > 0; cur = GET_SUCC(cur))
    {
        printf("i = %d, begin: %p, end: %p, pred: %p, succ: %p, size: %d\n",i++ , cur, FTRP(cur),
         GET(GET_PRED(cur)), GET(GET_SUCC(cur)), GET_SIZE(HDRP(cur)));
    }
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //advanced free list
    if((free_list = mem_sbrk(WSIZE * (4 + CLASS_SIZE))) == (void*)-1)
        return -1;
    
    PUT(free_list, 0);
    PUT(free_list + 1 * WSIZE, PACK(DSIZE, 1));
    PUT(free_list + 2 * WSIZE, PACK(DSIZE, 1));

    free_list += (2 * WSIZE);
    //free block divided by powers of 2, min size of free block is 16 bytes
    for(int i = 0; i < CLASS_SIZE; i ++)
        PUT(GET_FREE_LIST(i), NULL);

    PUT(GET_FREE_LIST(CLASS_SIZE), PACK(0, 1));

    return 0;
}

static void* extend_heap(size_t words)
{
    char* bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    //free block header and footer
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    //epilogue header
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return bp;
}

static int find_idx(size_t size)
{
    for(int i = 4; i <= 31; i ++)
        if(size <= (1 << i))
            return i - 4;
    return -1;
}

size_t roundUp2Pow(size_t num)
{
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;
    return num;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    char* bp;
    if(size == 0)
        return NULL;
    if(size <= DSIZE)
        asize = 2 * DSIZE;
    else 
    {
        size += DSIZE;
        asize = roundUp2Pow(size);
    }

    int idx = find_idx(asize);
    void* basePtr = GET_FREE_LIST(idx);
    if((void*)GET(basePtr) == NULL || GET_SIZE(HDRP(GET(basePtr))) == 0)
    {
        bp = extend_heap((asize + 8) * ENTRY_SIZE / WSIZE);
        if(bp == NULL) return NULL;

        char* cur = bp;
        size_t csize = asize * ENTRY_SIZE;
        while(csize >= asize)
        {
            PUT(HDRP(cur), PACK(asize, 0));
            PUT(FTRP(cur), PACK(asize, 0));
            cur = NEXT_BLKP(cur);
            PUT(HDRP(cur), PACK(csize - asize, 0));
            PUT(FTRP(cur), PACK(csize - asize, 0));
            csize -= asize;
        }

        PUT(basePtr, NEXT_BLKP(bp));
        PUT(GET_PRED(NEXT_BLKP(bp)), NULL);
        PUT(GET_SUCC(NEXT_BLKP(bp)), NEXT_BLKP(NEXT_BLKP(bp)));

        for(char* ptr = NEXT_BLKP(NEXT_BLKP(bp)); GET_SIZE(HDRP(ptr)) > 0; ptr = NEXT_BLKP(ptr))
        {
            PUT(GET_PRED(ptr), PREV_BLKP(ptr));
            PUT(GET_SUCC(ptr), NEXT_BLKP(ptr));
        }

        return bp;
    }
    else
    {
        bp = (char*)GET(GET_FREE_LIST(idx));
        PUT(basePtr, GET(GET_SUCC(bp)));
        PUT(GET_PRED(NEXT_BLKP(bp)), NULL);
        return bp;
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    size = roundUp2Pow(size);
    int idx = 0;
    idx = find_idx(size);
    void* bp = GET_FREE_LIST(idx);
    if((void*)GET(bp) == NULL || GET_SIZE(HDRP(GET(bp))) == 0)
    {
        PUT(GET_PRED(ptr), NULL);
        PUT(GET_SUCC(ptr), GET(bp));
        PUT(bp, (char*)ptr);
    }
    else
    {
        char* nxt = (char*)GET(bp);
        PUT(GET_PRED(ptr), NULL);
        PUT(GET_SUCC(ptr), nxt);
        PUT(GET_PRED(nxt), ptr);
        PUT(bp, (char*)ptr);
    }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t new_size)
{
    size_t copy_size = GET_SIZE(HDRP(ptr));
    void* new_ptr;
    if((new_ptr = mm_malloc(new_size)) == NULL)
        return NULL;
    if(new_size < copy_size) copy_size = new_size;
    memcpy(new_ptr, ptr, copy_size);
    mm_free(ptr);
    return new_ptr;
}
```

在 `mm_malloc` 申请内存时，如果对应的等价类中没有空闲块，**我们会向系统申请，并用链表将这块空间串起来**

### 测试

测试结果如下：

```c
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   93%    5694  0.000057100423
 1       yes   84%    5848  0.000057103139
 2       yes   92%    6648  0.000072 92205
 3       yes   90%    5380  0.000057 94885
 4       yes   11%   14400  0.000150 95681
 5       yes   74%    4800  0.000083 57692
 6       yes   75%    4800  0.000083 57485
 7       yes   34%   12000  0.000111107817
 8       yes   33%   24000  0.000184130152
 9       yes   10%   14401  0.030594   471
10       yes   16%   14401  0.001596  9021
Total          56%  112372  0.033045  3401

Perf index = 33 (util) + 40 (thru) = 73/100
```