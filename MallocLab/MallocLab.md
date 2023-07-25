# malloclab

- [malloclab](#malloclab)
  - [准备](#准备)
  - [实验开始](#实验开始)
    - [free\_list 基本思路](#free_list-基本思路)
      - [测试](#测试)
    - [segregate free list 基本思路](#segregate-free-list-基本思路)
      - [测试](#测试-1)
    - [结语](#结语)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

注：本实验无测试文件，我的 `trace` 测试文件是从 [这里](https://github.com/pgoodman/csc369/tree/master/malloclab/traces) 下载的

## 实验开始

> 注：本实验我最初的代码最终仅仅得分 `73/100`，后来我重新写了一个 `segregate free list`，最终得分为 `84/100`
>
> 文件中 `test.c` 为测试文件，编译时用 `gcc -g -m32 -o debug test.c mm.c memlib.c` 进行编译
>
> 另外，我们可以直接修改 `Makefile` 文件，将 `-O2` 改为 `-g`，这样便可以对 `mdriver` 进行调试
 
我最初的代码用 `free list` 表示，`segregate free list` 的代码用 `heap list` 表示

### free_list 基本思路

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

这个实现非常的简单，看代码就行

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
        //construct list in allocated area
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

#### 测试

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

### segregate free list 基本思路

对于 `segregate free list` 而言，我们首先维护一个 `HEAP_SIZE` 大小的数组，数组当中的元素代表对应请求空间大小的等价类，我们的等价类以 `2` 的幂的形式进行划分，也就是所有空闲块的大小分类为（我们最小的块大小为 `16 Bytes`）：

$$
[16],\ [17-32],\ [33-64],\ [65-128],\ \cdots,[1025-2048],\ \cdots
$$

> 数组中每个元素是一个指针，指向一个**双向**空闲块链表。我们将数组中每个下标对于的位置称为槽 `slots`

对于每次请求的大小 $n$，我们找到 $n$ 对应的区间，优先考虑该区间是否存在大小合适的空闲块，如果存在则直接分割，否则搜索下一个区间

每次归还一个块时，我们将该块加入空闲块链表中，如果该空闲块块存在前后相邻的两个**空闲块**，那么我们需要对空闲块进行合并

在对空闲块进行合并时，我们首先需要删除该空闲块的指针，也就是先将该空闲块从链表中删除，然后将合并后的空闲块加入链表。对于一个空闲块，不外乎四种情况：

* 该空闲块既**没有前驱**，也**没有后继**，说明该空闲块是数组对应槽位的第一个元素，那么将数组中对应的指针置空即可
* 该空闲块**有前驱**，**没有后继**，说明该空闲块是链表中的最后一个元素，我们**将链表中前一个元素的后继置空**（因为是双向链表）
* 该空闲块**没有前驱**，**有后继**，说明该空闲块是数组中对应槽位的第一个元素，我们将数组中对应的指针置位该空闲块的后继，并将此**后继的前驱置空**（同样是双向链表）
* 该空闲块**既有前驱**，**也有后继**，那么我们将该空闲块的前驱的后继指向该空闲块的后继，将该空闲块的后继的前驱指向该空闲块的前驱

每当我们需要插入一个空闲块时，首先找出该空闲块对应的槽位，此时分两类讨论：

* 对应槽位为空，也就是数组指针为空，我们直接将数组指针赋值为该空闲块的地址，并将空闲块的两个指针清空
* 对应槽位非空，我们对该空闲块的前驱清空，后继置为原先数组指针指向的空闲块，并将数组的指针重新赋值为当前空闲块的地址

需要说明的是，我们维护的是一个双向链表，因为我们除了对当前空闲块的指针进行清空外，还需要依据情况**对前驱和后继空闲块的两个指针进行清空**

完整代码如下（包含前面 `free list`）：

> 所有 `free list` 的函数均以 `free_list` 开头，所有 `segregate free list` 的函数均以 `heap_list` 开头

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
#define GET_SUCC(bp)    ((unsigned int*)((char*)bp + WSIZE))

//free list index
#define GET_FREE_LIST(num)  (free_list + WSIZE + ((num) * WSIZE))

#define GET_HEAP_LIST(num)  (heap_list + ((num) * WSIZE))

//the size of size class
#define CLASS_SIZE  30
//free list entries' size
//min val is 3
#define ENTRY_SIZE  3

#define HEAP_SIZE   20

//free list
static char* free_list;
//heap list
static char* heap_list;

static void* free_list_extend_heap(size_t words);
static void* heap_list_extend_heap(size_t words);
static void* coalesce(void* bp);
static int find_idx(size_t size);
size_t roundUp2Pow(size_t num);
void *mm_malloc(size_t size);
void *free_list_malloc(size_t size);
void* heap_list_malloc(size_t size);
void mm_free(void *ptr);
void free_list_free(void *ptr);
void heap_list_free(void* ptr);
void place(void* bp, size_t size);
void* find_fit(size_t size);

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
        printf("i = %d, begin: %p, end: %p, pred: %p, succ: %p, size: %d, alloc: %d\n",i++ , cur, FTRP(cur), 
        GET(GET_PRED(cur)), GET(GET_SUCC(cur)), GET_SIZE(HDRP(cur)), GET_ALLOC(HDRP(cur)));
    }
}

void Linklist(void* bp)
{
    int i = 0;
    for(void* cur = bp; cur; cur = GET(GET_SUCC(cur)))
    {
        printf("i = %d, begin: %p, end: %p, pred: %p, succ: %p, size: %d, alloc: %d\n",i++ , cur, FTRP(cur),
         GET(GET_PRED(cur)), GET(GET_SUCC(cur)), GET_SIZE(HDRP(cur)), GET_ALLOC(HDRP(cur)));
    }
}

void HeapPrint(void* bp)
{
    for(int i = 0; i < HEAP_SIZE; i ++)
    {
        if(GET(GET_HEAP_LIST(i)) != NULL)
            Linklist(GET(GET_HEAP_LIST(i)));
    }
} 

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //free list
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

    //heap list

    if((heap_list = mem_sbrk(WSIZE * (4 + HEAP_SIZE))) == (void*)-1)
        return -1;
    for(int i = 0; i < HEAP_SIZE; i ++)
        PUT(GET_HEAP_LIST(i), NULL);
    PUT(GET_HEAP_LIST(HEAP_SIZE), 0);
    PUT(GET_HEAP_LIST(HEAP_SIZE + 1), PACK(DSIZE, 1));
    PUT(GET_HEAP_LIST(HEAP_SIZE + 2), PACK(DSIZE, 1));
    PUT(GET_HEAP_LIST(HEAP_SIZE + 3), PACK(0, 1));

    if(heap_list_extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    return 0;
}

static void* heap_list_extend_heap(size_t words)
{
    char* bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    //free block header and footer
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    PUT(GET_PRED(bp), NULL);
    PUT(GET_SUCC(bp), NULL);
    //epilogue header
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

static void* free_list_extend_heap(size_t words)
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

//delete a free block from heap list
//we need delete all pointer points to this free block
//pay attention that this is a free block
static void delete_pointer(void* bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    size_t idx = find_idx(size);
    //this block don't have pred and succ, in other words, this is the first block in heap list
    if(GET(GET_PRED(bp)) == NULL && GET(GET_SUCC(bp)) == NULL)
    {
        PUT(GET_HEAP_LIST(idx), NULL);
    }
    //this block has succ, so its the first element in list
    else if(GET(GET_PRED(bp)) == NULL && GET(GET_SUCC(bp)) != NULL)
    {
        PUT(GET_HEAP_LIST(idx), GET(GET_SUCC(bp)));
        PUT(GET_PRED(GET(GET_SUCC(bp))), NULL);
        PUT(GET_SUCC(bp), NULL);
    }
    //the last element in list
    else if(GET(GET_PRED(bp)) != NULL && GET(GET_SUCC(bp)) == NULL)
    {
        PUT(GET_SUCC(GET(GET_PRED(bp))), NULL);
        //PUT(GET_SUCC(GET(GET_PRED(bp))), NULL);
        PUT(GET_PRED(bp), NULL);
    }
    else if(GET(GET_PRED(bp)) != NULL && GET(GET_SUCC(bp)) != NULL)
    {
        PUT(GET_SUCC(GET(GET_PRED(bp))), GET(GET_SUCC(bp)));
        PUT(GET_PRED(GET(GET_SUCC(bp))), GET(GET_PRED(bp)));
        PUT(GET_PRED(bp), NULL);
        PUT(GET_SUCC(bp), NULL);
    }

}

void insert_free_block(void* bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    size_t idx = find_idx(size);
    //corresponding slots is null
    if(GET(GET_HEAP_LIST(idx)) == NULL)
    {
        PUT(GET_HEAP_LIST(idx), bp);
        PUT(GET_PRED(bp), NULL);
        PUT(GET_SUCC(bp), NULL);
    }
    else
    {
        PUT(GET_SUCC(bp), GET(GET_HEAP_LIST(idx)));        
        PUT(GET_PRED(bp), NULL);        
        PUT(GET_PRED(GET(GET_HEAP_LIST(idx))), bp);
        PUT(GET_HEAP_LIST(idx), bp);
    }
}

static void* coalesce(void* bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if(prev_alloc && next_alloc)
    {
        if(GET_ALLOC(HDRP(bp)) == 0)
            insert_free_block(bp);
        return bp;
    }
    else if(prev_alloc && !next_alloc)
    {
        //remove free block from list
        delete_pointer(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc)
    {
        delete_pointer(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));        
        bp = PREV_BLKP(bp);        
    }
    else
    {
        delete_pointer(PREV_BLKP(bp));
        delete_pointer(NEXT_BLKP(bp));
        size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp))));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    if(GET_ALLOC(HDRP(bp)) == 0) 
        insert_free_block(bp);
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

void* find_fit(size_t asize)
{
    size_t idx = find_idx(asize);
    for(size_t i = idx; i < HEAP_SIZE; i ++)
    {
        void* cur = GET(GET_HEAP_LIST(i));
        while(cur)
        {
            if(GET_SIZE(HDRP(cur)) >= asize)
                return cur;
            cur = GET(GET_SUCC(cur));
        }
    }
    return NULL;
}

void place(void* bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    if((csize - asize) >= 2 * DSIZE)
    {
        delete_pointer(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        if(GET_ALLOC(HDRP(bp)) == 0)
            insert_free_block(bp);
    }
    else
    {
        delete_pointer(bp);        
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    //return free_list_malloc(size);
    return heap_list_malloc(size);
}

void* heap_list_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    if(size == 0)
        return NULL;
    if(size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    if((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = heap_list_extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

void *free_list_malloc(size_t size)
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
        bp = free_list_extend_heap((asize + 8) * ENTRY_SIZE / WSIZE);
        if(bp == NULL) return NULL;

        char* cur = bp;
        size_t csize = asize * ENTRY_SIZE;
        //construct list in allocated area
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
    //free_list_free(ptr);
    heap_list_free(ptr);
}

void heap_list_free(void* ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    PUT(GET_PRED(ptr), NULL);
    PUT(GET_SUCC(ptr), NULL);
    coalesce(ptr);
}

void free_list_free(void *ptr)
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

#### 测试

我们仅对 `segregate free list` 进行测试，结果如下：

```bash
$ ./mdriver -v -t ./traces/
Team Name:Atom
Member 1 :Nishiki Chisato:NishikiChisato@outlook.com
Using default tracefiles in ./traces/
Measuring performance with gettimeofday().

Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   98%    5694  0.000211 26947
 1       yes   98%    5848  0.000194 30129
 2       yes   97%    6648  0.000253 26277
 3       yes   99%    5380  0.000187 28755
 4       yes   65%   14400  0.000316 45555
 5       yes   93%    4800  0.000277 17347
 6       yes   90%    4800  0.000276 17360
 7       yes   55%   12000  0.000301 39814
 8       yes   51%   24000  0.000563 42667
 9       yes   25%   14401  0.031962   451
10       yes   29%   14401  0.002123  6782
Total          73%  112372  0.036664  3065

Perf index = 44 (util) + 40 (thru) = 84/100
```

### 结语

这个实验做了我 `28.5` 个小时，一开始我思路方向错了，导致花费了一半的时间写 `free list`，后面一半的时间才重新看回书，开始写 `segregate free list`。说实话，整个调试的过程我非常的痛苦，我一直在用 `gdb` 不断地调试，最终才把代码逐渐改对

在写 `segregate` 的时候，最终有一个 `bug` 导致我代码不断地出错，我一直找不到，这个地方也是我耗时最长的，我一直不理解为什么一个已经分配的块会存在于数组当中。最后我不断地从内存往回推，这才发现是双向链表需要对当前节点的前驱和后继的指针也需要进行清空

回顾整个坐牢的过程，我发现这才是我进步最大的时候，这个过程中我需要不断地去思考我的代码到底哪里出来问题，需要不断地倒推。除吃之外我还学到了非常多的 `gdb` 的调试技巧，虽然花了很多时间，但感觉很不错