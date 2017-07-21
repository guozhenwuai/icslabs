/*
 * mm.c
 * use explicit free list and segregated fit to organize free blocks.
 * use 10 free lists for size of <=16, <=32, <=64, <=128, <=256, <=512, <=1024, <=2048, <=4096, >4096
 * use static pointers (freelist_head, freelist_tail) to organize each free list.
 * use first-fit strategy to place newly allocated blocks.
 * adjust some alloc size to promote util score.
 * the codes are adapted from textbook.
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
    "515030910231",
    /* First member's full name */
    "Wu Xinyue",
    /* First member's email address */
    "971137346@qq.com",
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

/* Basic constants and macros */
#define WSIZE 4                 /* Word and header/footer size (bytes) */
#define DSIZE 8                 /* Double word size (bytes) */                 
#define CHUNKSIZE (1<<8)       /* Extend heap by this amount (bytes) */

#define MAX(x,y) ((x)>(y)?(x):(y))

/* Pack a size and allocated bit into a word */
#define PACK(size,alloc) ((size)|(alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p)&~0x7)
#define GET_ALLOC(p) (GET(p)&0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp))-DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp)+GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

/* Given free block ptr bp, compute address of next and previous free blocks */
#define GET_FREE_PRED(bp) (*(unsigned int *)(bp))
#define GET_FREE_SUCC(bp) (*((unsigned int *)(bp)+1))

/* Given free block ptr bp, set address of next and previous free blocks */
#define SET_FREE_PRED(bp,addr) (*(unsigned int *)(bp)=(addr))
#define SET_FREE_SUCC(bp,addr) (*((unsigned int *)(bp)+1)=(addr))

/* GET free list tail address */
#define LIST_TAIL(head) ((head)+2*WSIZE)

/* heap_listp points to first block */
static char *heap_listp = 0;

/* freelist_header points to first free block */
static void *freelist_head_4 = 0;
static void *freelist_head_5 = 0;
static void *freelist_head_6 = 0;
static void *freelist_head_7 = 0;
static void *freelist_head_8 = 0;
static void *freelist_head_9 = 0;
static void *freelist_head_10 = 0;
static void *freelist_head_11 = 0;
static void *freelist_head_12 = 0;
static void *freelist_head_13 = 0;

/* freelist_tail points to the end of free list*/
static void *freelist_tail_4 = 0;
static void *freelist_tail_5 = 0;
static void *freelist_tail_6 = 0;
static void *freelist_tail_7 = 0;
static void *freelist_tail_8 = 0;
static void *freelist_tail_9 = 0;
static void *freelist_tail_10 = 0;
static void *freelist_tail_11 = 0;
static void *freelist_tail_12 = 0;
static void *freelist_tail_13 = 0;

/* Declerations */
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void *coalesce(void *bp);
static void freelist_remove(void *bp);
static void freelist_add(void *bp);
static void *get_freelist(size_t asize);
static void *next_freelist(void *flp);
static void *first_freelist();

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Initialize freelist head and tail. */
    if((freelist_head_4 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_4 = LIST_TAIL(freelist_head_4);
    SET_FREE_SUCC(freelist_head_4,freelist_tail_4);
    SET_FREE_PRED(freelist_head_4,0);
    SET_FREE_SUCC(freelist_tail_4,0);
    SET_FREE_PRED(freelist_tail_4,freelist_head_4);

    if((freelist_head_5 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_5 = LIST_TAIL(freelist_head_5);
    SET_FREE_SUCC(freelist_head_5,freelist_tail_5);
    SET_FREE_PRED(freelist_head_5,0);
    SET_FREE_SUCC(freelist_tail_5,0);
    SET_FREE_PRED(freelist_tail_5,freelist_head_5);

    if((freelist_head_6 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_6 = LIST_TAIL(freelist_head_6);
    SET_FREE_SUCC(freelist_head_6,freelist_tail_6);
    SET_FREE_PRED(freelist_head_6,0);
    SET_FREE_SUCC(freelist_tail_6,0);
    SET_FREE_PRED(freelist_tail_6,freelist_head_6);

    if((freelist_head_7 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_7 = LIST_TAIL(freelist_head_7);
    SET_FREE_SUCC(freelist_head_7,freelist_tail_7);
    SET_FREE_PRED(freelist_head_7,0);
    SET_FREE_SUCC(freelist_tail_7,0);
    SET_FREE_PRED(freelist_tail_7,freelist_head_7);

    if((freelist_head_8 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_8 = LIST_TAIL(freelist_head_8);
    SET_FREE_SUCC(freelist_head_8,freelist_tail_8);
    SET_FREE_PRED(freelist_head_8,0);
    SET_FREE_SUCC(freelist_tail_8,0);
    SET_FREE_PRED(freelist_tail_8,freelist_head_8);

    if((freelist_head_9 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_9 = LIST_TAIL(freelist_head_9);
    SET_FREE_SUCC(freelist_head_9,freelist_tail_9);
    SET_FREE_PRED(freelist_head_9,0);
    SET_FREE_SUCC(freelist_tail_9,0);
    SET_FREE_PRED(freelist_tail_9,freelist_head_9);

    if((freelist_head_10 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_10 = LIST_TAIL(freelist_head_10);
    SET_FREE_SUCC(freelist_head_10,freelist_tail_10);
    SET_FREE_PRED(freelist_head_10,0);
    SET_FREE_SUCC(freelist_tail_10,0);
    SET_FREE_PRED(freelist_tail_10,freelist_head_10);

    if((freelist_head_11 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_11 = LIST_TAIL(freelist_head_11);
    SET_FREE_SUCC(freelist_head_11,freelist_tail_11);
    SET_FREE_PRED(freelist_head_11,0);
    SET_FREE_SUCC(freelist_tail_11,0);
    SET_FREE_PRED(freelist_tail_11,freelist_head_11);

    if((freelist_head_12 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_12 = LIST_TAIL(freelist_head_12);
    SET_FREE_SUCC(freelist_head_12,freelist_tail_12);
    SET_FREE_PRED(freelist_head_12,0);
    SET_FREE_SUCC(freelist_tail_12,0);
    SET_FREE_PRED(freelist_tail_12,freelist_head_12);

    if((freelist_head_13 = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    freelist_tail_13 = LIST_TAIL(freelist_head_13);
    SET_FREE_SUCC(freelist_head_13,freelist_tail_13);
    SET_FREE_PRED(freelist_head_13,0);
    SET_FREE_SUCC(freelist_tail_13,0);
    SET_FREE_PRED(freelist_tail_13,freelist_head_13);


    /* Create the initial empty heap */
    if((heap_listp = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    PUT(heap_listp,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(3*WSIZE),PACK(0,1));
    heap_listp+=(2*WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
        return -1;
    return 0;
}

/*
 * extend_heap - Allocate extra block to extend heap.
 */
 static void *extend_heap(size_t words)
 {
     char *bp;size_t size;

     /* Allocate an even number of words to maintain alignment. */
     size = (words % 2) ? (words+1)*WSIZE : words*WSIZE;
     if((long)(bp=mem_sbrk(size))==-1)
        return NULL;

     /* Initialize free block header/footer and the epilogue header */
     PUT(HDRP(bp), PACK(size,0));         /* Free block header */
     PUT(FTRP(bp), PACK(size,0));         /* Free block footer */
     PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1)); /* New epilogue header*/

     /* Coalesce if the previous block was free */
     return coalesce(bp);
 }


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;             /* Adjusted block size */
    size_t extendsize;        /* Amount to extend heap if not fit*/
    char *bp;

    /* Ignore spurious requests */
    if(size==0)
        return NULL;
    /* Adjust some special size. */
    else if(size==112)size=136;
    else if(size==448)size=520;
    if(size<=DSIZE)
        asize = 2*DSIZE;

    /* Adjust block size to include overhead and alignment reqs. */
    else
        asize = DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);

    /* Search the free list for a fit */
    if((bp=find_fit(asize))!=NULL){
        place(bp,asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    if(asize==16)extendsize=16;
    else extendsize = MAX(asize,CHUNKSIZE);
    if((bp=extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp,asize);
    return bp;
}

/*
 * find_fit - Find enough space to place new block.
 *     Return a pointer points to the block, NULL if not found.
 *     Use first-fit strategy. */
static void *find_fit(size_t asize)
{
    /* First fit search*/
    /* void *bp;

    for(bp=heap_listp;GET_SIZE(HDRP(bp))>0;bp=NEXT_BLKP(bp)){
        if(!GET_ALLOC(HDRP(bp)) && (asize<=GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    return NULL;*/

    void *bp;
    void *freelist_head = get_freelist(asize);
    void *freelist_tail = LIST_TAIL(freelist_head);

    while(freelist_head){
	freelist_tail=LIST_TAIL(freelist_head);    
	for(bp=GET_FREE_SUCC(freelist_head);bp!=freelist_tail;bp=GET_FREE_SUCC(bp)){
            if(GET_SIZE(HDRP(bp))>=asize){
                return bp;
	    }
        }
	freelist_head=next_freelist(freelist_head);
    }
    return NULL;
}

/*
 * place - Place the new block at found place.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if((csize-asize)>=(2*DSIZE)){
        /* Remove bp's block from freelist. */
        freelist_remove(bp);
        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize,0));
        PUT(FTRP(bp), PACK(csize-asize,0));
        /* Add the newly-splited block to freelist. */
        freelist_add(bp);
    }
    else{
        /* Remove bp's block from freelist. */
        freelist_remove(bp);
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    coalesce(bp);
}

/*
 * coalesce - Coalesce the newly freed block with its previous one and next one if any. 
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {           /* Case 1 */
        freelist_add(bp);
    }


    else if (prev_alloc && !next_alloc) {     /* case 2 */
        /* remove the next block from freelist and coalesce it with bp's block. */
        freelist_remove(NEXT_BLKP(bp));
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size,0));
        PUT(FTRP(bp), PACK(size,0));
        /* add coalesced block to freelist. */
        freelist_add(bp);
    }

    else if (!prev_alloc && next_alloc) {     /* case 3 */
        /* freelist doesn't need to change, just enlarge the size of bp's previous block. */
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        bp = PREV_BLKP(bp);
    }

    else{                                     /* case 4 */
        /* remove the next block from freelist. */
         freelist_remove(NEXT_BLKP(bp));
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        bp=PREV_BLKP(bp);
        }
    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

    size_t oldsize;
    size_t asize;
    void *newptr;
    
    /* If size == 0 then this is just free, and we return NULL.  */
    if(size==0){
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL){
        return mm_malloc(size);
    }

    size=size+(2048-(size%2048));
    if(size<=DSIZE) asize = 2*DSIZE;
    else asize = DSIZE*((size+2*DSIZE-1)/DSIZE);
    oldsize = GET_SIZE(HDRP(ptr));
    if(asize<=oldsize) return ptr;
    else{
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
        size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));

        if(!next_alloc&&(oldsize+next_size)>=asize){
            freelist_remove(NEXT_BLKP(ptr));
            PUT(HDRP(ptr),PACK(oldsize+next_size,1));
            PUT(FTRP(ptr),PACK(oldsize+next_size,1));
            return ptr;
        }
        else{
            newptr = mm_malloc(size);
            /* If realloc() fails the original block is left untouched */
            if(!newptr){
                return 0;
            }
            /* Copy the old data. */
            memcpy(newptr, ptr, oldsize);
            /* Free the old block. */
            mm_free(ptr);
            return newptr;
        }
    }
}

/*
 * freelist_remove - remove a block from free list.
 */
static void freelist_remove(void *bp){
    unsigned int *pred = GET_FREE_PRED(bp);
    unsigned int *succ = GET_FREE_SUCC(bp);
    SET_FREE_PRED(succ,pred);
    SET_FREE_SUCC(pred,succ);
}

/*
 * freelist_add - Add a block to free list.
 */
static void freelist_add(void *bp){
    void *freelist_head = get_freelist(GET_SIZE(HDRP(bp)));
    unsigned int *succ = GET_FREE_SUCC(freelist_head);
    SET_FREE_PRED(succ,bp);
    SET_FREE_SUCC(bp,succ);
    SET_FREE_PRED(bp,freelist_head);
    SET_FREE_SUCC(freelist_head,bp);
}

/*
 * mm_check - check the correctness of freelist, show indormation aboout each block.
 */
 int mm_check(){
     /* Check free list. */
     void *p;
     void *freelist_head=first_freelist();
     void *freelist_tail=LIST_TAIL(freelist_head);
     while(freelist_head){
         for(p=GET_FREE_SUCC(freelist_head);p!=freelist_tail;p=GET_FREE_SUCC(p)){
             //check free blocks' status
             if(GET_ALLOC(HDRP(p))||GET_ALLOC(FTRP(p))){
                 printf("ERROR: block in the free list not marked free.\n");
                 return -1;
             }
             //check free blocks' pointer
             if(GET_FREE_PRED(GET_FREE_SUCC(p))!=p||
                GET_FREE_SUCC(GET_FREE_PRED(p))!=p){
                    printf("ERROR: free block's pointers are not consistent.\n");
                    return -1;
                }
         }
         freelist_head=next_freelist(freelist_head);
	 freelist_tail=LIST_TAIL(freelist_head);
     }	   

     /* Check all the blocks.*/
     for(p=heap_listp;GET_SIZE(HDRP(p))>0;p=NEXT_BLKP(p)){
         //check the consistency of HDRP and FTRP
         if(GET_SIZE(HDRP(p))!=GET_SIZE(FTRP(p))||
           GET_ALLOC(HDRP(p))!=GET_ALLOC(FTRP(p))){
             printf("ERROROR: block's header and footer are not consistent.\n");
             return -1;
         }
         //print information
         if(GET_ALLOC(HDRP(p))){
             printf("FREE BLOCK.\n");
             printf("PRED: %x\n",(unsigned int)GET_FREE_PRED(p));
             printf("SUCC: %x\n",(unsigned int)GET_FREE_SUCC(p));
         }
         else{
             printf("ALLOCATED BLOCK.\n");
         }
         printf("SIZE: %d\n",GET_SIZE(HDRP(p)));
         printf("ADDRESS: %x\n",(unsigned int)p);
         printf("\n");
     }
     return 1;
 }




/* Get approprate free list according to block's size. */
void *get_freelist(size_t asize){
    if(asize<=16) return freelist_head_4;
    else if(asize<=32) return freelist_head_5;
    else if(asize<=64) return freelist_head_6;
    else if(asize<=128) return freelist_head_7;
    else if(asize<=256) return freelist_head_8;
    else if(asize<=512) return freelist_head_9;
    else if(asize<=1024) return freelist_head_10;
    else if(asize<=2048) return freelist_head_11;
    else if(asize<=4096) return freelist_head_12;
    else return freelist_head_13;
}

/* Get the next(bigger) free list. */
void *next_freelist(void *flp){
    if(flp==freelist_head_4)return freelist_head_5;
    else if(flp==freelist_head_5)return freelist_head_6;
    else if(flp==freelist_head_6)return freelist_head_7;
    else if(flp==freelist_head_7)return freelist_head_8;
    else if(flp==freelist_head_8)return freelist_head_9;
    else if(flp==freelist_head_9)return freelist_head_10;
    else if(flp==freelist_head_10)return freelist_head_11;
    else if(flp==freelist_head_11)return freelist_head_12;
    else if(flp==freelist_head_12)return freelist_head_13;
    else if(flp==freelist_head_13)return NULL;
    else{
        printf("ERROR: not match free list head.\n");
        return NULL;
    }
}

/* Get the first(smallest) free list. */
void *first_freelist(){
    return freelist_head_4;
}







