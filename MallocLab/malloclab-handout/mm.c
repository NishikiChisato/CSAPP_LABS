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

#define GET_PRED(bp)    ((unsigned int*)(GET(bp)))
#define GET_SUCC(bp)    ((unsigned int*)(GET(bp + WSIZE)))

#define GET_FREE_LIST(num)  (free_list + WSIZE + (num * WSIZE))

//the size of size class
#define CLASS_SIZE  20
//free list entries' size
#define ENTRY_SIZE  5

static char* free_list;

static void* extend_heap(size_t words);
static void* coalesce(void* bp);
static void* find_fit(void* base_ptr, size_t size);
static void place(void* bp, size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((free_list = mem_sbrk(WSIZE * (4 + CLASS_SIZE))) == (void*)-1)
        return -1;
    
    PUT(free_list, 0);
    PUT(free_list + 1 * WSIZE, PACK(DSIZE, 1));
    PUT(free_list + 2 * WSIZE, PACK(DSIZE, 1));
    //PUT(free_list + 3 * WSIZE, PACK(0, 1));

    free_list += (2 * WSIZE);
    //free block divided by powers of 2, min size of free block is 16 bytes
    //[16], [17, 32], [33, 64], ...
    for(int i = 0; i < CLASS_SIZE; i ++)
        PUT(GET_FREE_LIST(i), NULL);

    //PUT(free_list + WSIZE + CLASS_SIZE * WSIZE, PACK(0, 1));

    PUT(GET_FREE_LIST(CHUNKSIZE), PACK(0, 1));

    //if(extend_heap(CHUNKSIZE / WSIZE) == NULL)
    //    return -1;
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

//bp point to free block
static int find_idx(size_t size)
{
    int idx = 0;
    for(int i = 3; i <= 31; i ++)
        if(size <= (1 << i))
            idx = i;
    return idx;
}

//insert a free block in head
// static void insert(void* bp)
// {
//     size_t size = GET_SIZE(bp);
//     int idx = find_idx(size);
//     if(GET(GET_FREE_LIST(idx)) == NULL)
//     {
//         PUT(GET_FREE_LIST(idx), bp);
//         PUT(GET_PRED(bp), NULL);
//         PUT(GET_SUCC(bp), NULL);
//     }
//     else
//     {
//         void* entryAddr = GET(GET_FREE_LIST(idx));
//         PUT(GET_SUCC(bp), entryAddr);
//         PUT(GET_PRED(entryAddr), bp);
//         PUT(GET_PRED(bp), NULL);
//     }
// }

//two free block can be coalesced if and only if they are adjacent
static void* coalesce(void* bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if(prev_alloc && next_alloc)
        return bp;
    else if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));   
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize, extendsize;
    char* bp;
    if(size == 0)
        return NULL;
    if(size <= DSIZE)
        asize = 2 * DSIZE;
    else asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);

    int idx = find_idx(size);
    void* basePtr = GET_FREE_LIST(idx);
    if((void*)GET(basePtr) == NULL)
    {
        bp = extend_heap(((1 << idx) + 8) * 20 / WSIZE);

        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        PUT(basePtr, NEXT_BLKP(bp));
        PUT(GET_PRED(NEXT_BLKP(bp)), NULL);
        PUT(GET_SUCC(NEXT_BLKP(bp)), NEXT_BLKP(NEXT_BLKP(bp)));
        for(void* cur = NEXT_BLKP(GET(basePtr)); GET_SIZE(HDRP(cur)) > 0; cur = NEXT_BLKP(cur))
        {
            PUT(GET_PRED(cur), PREV_BLKP(cur));
            PUT(GET_SUCC(cur), NEXT_BLKP(cur));
        } 
        return bp;
    }
    else
    {
        bp = GET(GET_FREE_LIST(idx));
        PUT(GET_FREE_LIST(idx), NEXT_BLKP(bp));
        return bp;
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    // size_t size = GET_SIZE(HDRP(ptr));
    // PUT(HDRP(ptr), PACK(size, 0));
    // PUT(FTRP(ptr), PACK(size, 0));
    // coalesce(ptr);
    size_t size = GET_SIZE(ptr);
    int idx = 0;
    idx = find_idx(size);
    void* bp = GET_FREE_LIST(idx);
    PUT(GET_PRED(ptr), NULL);
    PUT(GET_SUCC(ptr), GET(bp));
    PUT(bp, ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t old_size = GET_SIZE(HDRP(ptr));
    if(size < old_size) old_size = size;
    void* new_ptr;
    if((new_ptr = mm_malloc(size)) == NULL)
        return NULL;
    memcpy(new_ptr, ptr, old_size);
    mm_free(ptr);
    return new_ptr;
}














