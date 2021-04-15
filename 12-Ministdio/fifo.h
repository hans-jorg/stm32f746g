#ifndef FIFO_H
#define FIFO_H
/**
 *  @file   fifo.h
 */


/**
 *  @brief  Data structure to store info about a fifo, including its data
 *
 * @note    Uses x[0] hack. This structure is a header
 * @note    First element is a pointer to force data alignement
 */

typedef struct fifo_s {
    char    *front;             // pointer to first char in fifo
    char    *rear;              // pointer to last char in fifo
    int     size;               // number of char stored in fifo
    int     capacity;           // number of chars in data
    char    data[];             // flexible array
} FIFO_t;

typedef FIFO_t *FIFO;

#define DECLARE_FIFO_AREA(AREANAME,SIZE) unsigned AREANAME[ \
                        (sizeof(struct fifo_s)+(SIZE)+sizeof(unsigned)-1)/sizeof(unsigned) \
                        ]

FIFO    fifo_init(void *area,int size);
void    fifo_deinit(FIFO f);
int     fifo_insert(FIFO f, char x);
int     fifo_remove(FIFO f);
void    fifo_clear(FIFO f);

#define fifo_capacity(F) ((F)->capacity)
#define fifo_size(F) ((F)->size)
#define fifo_empty(F) ((F)->size==0)
#define fifo_full(F) ((F)->size==fifo_capacity(F))

#endif
