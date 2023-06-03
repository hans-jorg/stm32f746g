/**
 * @file    fifo.c
 *
 * @note    FIFO for chars
 * @note    Uses a global data defined by DECLARE_fifo_AREA macro
 * @note    It does not use malloc
 * @note    Size must be defined in DECLARE_fifo_AREA and in fifo_init (Ugly)
 * @note    Uses as many dependencies as possible
 */

#include "fifo.h"


/**
 * @brief   initializes a fifo area
 */

FIFO
fifo_init(void *b, int n) {
FIFO f = (FIFO) b;

    f->front = f->rear = f->data;
    f->size = 0;
    f->capacity = n;
    return f;
}

/**
 * @brief   Clears fifo
 *
 * @note    Does not free any area, because it is static
            In future, it will free area
 */

void
fifo_deinit(FIFO f) {

    f->size = 0;
    f->front = f->rear = f->data;

}

/**
 * @brief   Clears fifo
 *
 * @note    Does not free area. For now identical to deinit
 */
 void
 fifo_clear(FIFO f) {

    f->size = 0;
    f->front = f->rear = f->data;

}

/**
 * @brief   Insert an element in fifo
 *
 * @note    return -1 when full
 */

int
fifo_insert(FIFO f, char x) {

    if( fifo_full(f) )
        return -1;

    *(f->rear++) = x;
    f->size++;
    if( (f->rear - f->data) > f->capacity )
        f->rear = f->data;
    return 0;
}

/**
 * @brief   Removes an element from fifo
 *
 * @note    return -1 when empty
 */

int
fifo_remove(FIFO f) {
char ch;

    if( fifo_empty(f) )
        return -1;

    ch = *(f->front++);
    f->size--;
    if( (f->front - f->data) > f->capacity )
        f->front = f->data;
    return ch;
}
