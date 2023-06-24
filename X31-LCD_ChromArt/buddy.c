/**
 *  @file   buddy.c
 *
 *  @note   Memory allocator using buddy allocator with bit vectors
 *
 *
 *  Level   |    Indices
 *  --------|---------------------
 *     0    |    0
 *     1    |    1-2
 *     2    |    3-4 * 5-6
 *     3    |    7-8 * 9-10 * 11-12 * 13-14
 *     4    |   15-16 * 17-18 * 19-20 * 21-22 * 23-24 * 25-26 * 27-28 * 29-30
 *
 *  @note
 *    All blocks at a level n can be found between in the range
 *         2^n - 1 to  2^{n+1}-2
 *
 *  @note
 *    To find the ancestor of a node k, subtract 1 and divide by 2, i.e.
 *             antecessor(k) = {k-1} over {2}
 *
 *  @note
 *    To find the successor of a node k, calculate 2*k+1  and   2*k + 2
 *
 *  @note
 *    All right leaves have even indices and all left leaves are odd.
 *
 *  @note
 *    The allocation is governed by two bits: used and split. The used bit set indicates that
 *    this block is full allocated. The split bit indicate that it has been split and allocation
 *    is done further below.
 *
 *    When a block is used and its buddy too, the parent block used bit must be set.
 *
 *    When a block is set free and its buddy remains used, the parent block used bit must
 *      be cleared.
 *
 *    When a block is set free and its buddy is already free, the parent block split bit must
 *      be cleared.
 *
 *    By observing the two bits, one can determine its status.
 *
 */

#include <stdint.h>
#ifdef DEBUG
#include <stdio.h>
#include <string.h>
#endif


#include "bitvector.h"
#include "buddy.h"

/**
 *  @brief  pool->mapsizeMAX
 *
 *  Define the bitmap size used to manage the allocation process
 *
 *  @note   It limits the ratio POLL_SIZE/POLL_MINSIZE
 */
#define  MAXRATIO   2048

#define  MAPSIZEMAX   (MAXRATIO*2)

/**
 *  @brief  Buddy area pool
 *
 *  @note   There is only one pool!!!
 */
typedef struct {
    char        *baseaddress;                   /// base address of area to be managed
    long        size;                           /// size of area to be managed (=power of 2)
    long        minimalsize;                    /// minimal block size
    long        mapsize;                        /// size/minimalsize
    long        treesize;                       /// pool->mapsize*2-1
    BV_TYPE     used[MAPSIZEMAX];               /// bit vector to store free(0) or used(1) block
    BV_TYPE     split[MAPSIZEMAX];              /// bit vector to signal if a block was split
} POOL_t;

/**
 *  @brief  Buddy area
 */
///@{
static POOL_t   poolarea;
static POOL_t   *pool = &poolarea;
///@}
#define TREESIZE       (pool->mapsize*2-1)                    ///< Number of elements in the tree

/**
 *  @brief  Structure used to navigate the allocation tree
 */

typedef struct {
    int         level;      ///< level of node
    int         index;      ///< index of node
    int         size;       ///< size of block
    uint32_t    addr;       ///< address of block
} nodeinfo;


/**
 *  @brief  buddy_init
 */
int
Buddy_Init(char *address, long size, long minsize) {

    if( size/minsize > MAXRATIO )
        return -1;

    pool->baseaddress = address;                /// base address of area to be managed
    pool->size        = size;                   /// size of area to be managed (=power of 2)
    pool->minimalsize = minsize;                /// minimal block size
    pool->mapsize     = size/minsize;           /// size/minimalsize
    pool->treesize    = 2*pool->mapsize-1;      /// pool->mapsize*2-1

    bv_clearall(pool->used,pool->mapsize*2);    /// Clear used block flags
    bv_clearall(pool->split,pool->mapsize*2);   /// Clear split block flags

    return 0;
}

/**
 *  @brief  buddy_alloc
 */
void *
Buddy_Alloc(unsigned size) {
int level;
int s;
int k;
int l;
uint32_t a;

nodeinfo stack[MAXRATIO];
int sp;
nodeinfo node;

    // Too big?
    if( size > pool->size )
        return 0;

    // Already full
    if( bv_test(pool->used,0) )
        return 0;

    sp = 0;
    stack[sp].level = 0;
    stack[sp].index = 0;
    stack[sp].size = pool->size;
    stack[sp].addr = 0;
    sp++;

    while( sp > 0 ) {
        sp--;
        node = stack[sp];
        k = node.index;
        s = node.size;
        a = node.addr;
        l = node.level;

        // test if block already used
        if( bv_test(pool->used,k) )
            continue;
        // test if need full block
        if( (size > s/2) || (s == pool->minimalsize) ) {
            // if already split, try another block
            if( bv_test(pool->split,k) == 0 ) {
                // reserve it
                bv_set(pool->used,k);
                return (void *) ((char *) pool->baseaddress+a);
            }
        }
        s /= 2;
        if( size > s )
            continue;

        // Mark as split
        bv_set(pool->split,k);
        // Try left and right leaves.
        l++;
        //Left must be on top of stack
        stack[sp].index = 2*k+2;
        stack[sp].addr  = a+s;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;
        stack[sp].index = 2*k+1;
        stack[sp].addr  = a;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;
    }
    return 0;
}

static inline int isodd(int n) { return n&1; }
static inline int iseven(int n) { return (n&1)^1; }

/**
 *  @brief  buddy_free
 */
void Buddy_Free(void *addr) {
uint32_t disp = (char *) addr - (char *)pool->baseaddress;       // 4 GB limit
int b,d,k,p;

    d = disp/pool->minimalsize;

    k = pool->mapsize+d-1;
    // Free if it is not
    bv_clear(pool->used,k);
    bv_clear(pool->split,k);
    // Find block to be freed
    while( k > 0 ) {
            k /= 2;
        if( bv_test(pool->used,k) ) {
            bv_clear(pool->used,k);
            bv_clear(pool->split,k);
            break;
        }
    }
    // Adjust parents
    while( k > 0 ) {
        // find buddy

        if( isodd(k) )
            b = k+1;
        else
            b = k-1;
        if(  (bv_test(pool->used,k)==0)
           &&(bv_test(pool->used,b)==0)
           &&(bv_test(pool->split,k)==0)
           &&(bv_test(pool->split,b))    ) {
            p = k/2;
            bv_clear(pool->split,p);
        }
        k /= 2;
    }
}



#ifdef DEBUG

/**
 *  @brief  fillmap
 */
static void
fillmap(char *m,int start, int end, char c) {
int i;

    for(i=start;i<end;i++) {
        if( c == '-' || m[i] == '-' )
            m[i] = c;
        else
            m[i] = '*';
    }

}


/**
 *  @brief  buildmap
 */
static void
buildmap(char *m) {
int level;
int s;
int k;
int l;
uint32_t a;
nodeinfo stack[pool->mapsize];
int sp;
nodeinfo node;

    fillmap(m,0,pool->mapsize,'-');

    sp = 0;
    stack[sp].level = 0;
    stack[sp].index = 0;
    stack[sp].size = pool->size/pool->minimalsize;
    stack[sp].addr = 0;
    sp++;

    while(sp>0) {
        sp--;
        node = stack[sp];
        k = node.index;
        s = node.size;
        a = node.addr;
        l = node.level;
        // test if block already used
        if( bv_test(pool->used,k) ) {
            fillmap(m,a,a+s,'U');
        }

        if( s == 1 )
            continue;

        s /= 2;
        // Try left and right leaves.
        l++;
        //Left must be on top of stack
        stack[sp].index = 2*k+2;
        stack[sp].addr  = a+s;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;
        stack[sp].index = 2*k+1;
        stack[sp].addr  = a;
        stack[sp].size  = s;
        stack[sp].level = l;
        sp++;

    }

    m[pool->mapsize] = '\0';
}


/**
 *  @brief  print allocation map
 */
void Buddy_PrintMap(void) {
char map[pool->mapsize+1];
    buildmap(map);
    printf("|%s|\n",map);
}



void Buddy_PrintAddresses(void) {
int level;
int k;
int lim;
uint32_t addr;
uint32_t size;
int delta;

    level = 0;
    size = pool->size;
    lim = 0;
    addr = 0;
    delta = 1;
    for(k=0;k<TREESIZE;k++) {
        printf("level = %-2d node = %-3d address = %08X  size=%08X\n",level,k,addr,size);
        if( k == lim ) {
            level++;
            delta *= 2;
            lim += delta;
            addr = 0;
            size /= 2;
            putchar('\n');
        } else {
            addr += size;
        }
    }
}

#endif

