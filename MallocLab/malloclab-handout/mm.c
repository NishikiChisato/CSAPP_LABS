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
#define NEXT_BLKP(bp)   ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))
#define PREV_BLKP(bp)   ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))

//the size of size class
//[1], [2], [3, 4], [5, 8], ..., [256k + 1, 512k]
#define CLASS_SIZE  20
//free list entries' size
#define ENTRY_SIZE  5

static char* free_list = NULL;

static void* extend_heap(size_t words);
static void* coalesce(void* bp);
static void* find_fit(size_t size);
static void* place(void* bp, size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((free_list = mem_sbrk(WSIZE * (CLASS_SIZE + 1))) == (void*)-1)
        return -1;
    PUT(free_list, PACK(0, 1));
    free_list += WSIZE;
    //every entries in free list is ptr
    for(int i = 0; i < CLASS_SIZE; i ++)
        PUT(free_list + i, NULL);
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

static void* extend_heap(size_t words)
{
    char* bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((bp = mem_sbrk(size) == (void*)-1))
        return NULL;
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

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
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        size += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));   
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

    if((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

int lowbit(int x)
{
    return x & -x;
}

//size >= 2 * DSIZE
static void* find_fit(size_t size)
{
    size_t bit = 0, max_bit = 0, sz = size;
    size_t idx = 0;
    void* bp;
    while(sz)
    {
        bit = lowbit(sz);
        sz -= bit;
        max_bit = MAX(max_bit, bit);
    }
    max_bit <<= 1;
    for(size_t i = 1; i <= max_bit; i <<= 1)
        idx++;
    if((free_list + idx) == NULL)
    {
        if(free_list[idx] = extend_heap(ENTRY_SIZE) == NULL)
            return NULL;
    }
    for(bp = free_list[idx]; GET_SIZE(bp) > 0; bp = NEXT_BLKP(bp))
    {
        if(!GET_ALLOC(bp) && (size <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL;
}

static void* place(void* bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    if((csize - asize) >= (2 * DSIZE))
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(HDRP(bp), PACK(csize - asize, 0));
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














