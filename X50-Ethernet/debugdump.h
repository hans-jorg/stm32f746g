#ifndef DEBUGDUMP_H
#define DEBUGDUMP_H
/**
 * @file    debugdump.h
 * 
 * @note    routines to dump memory
 */

/**
 * @brief   hexdump
 *
 * @note    print a memory dump
 */
static inline void
hexdump(void *area, int size, unsigned addr) {
unsigned offset;
unsigned a = addr;
unsigned char *c;
int i;

    for(offset=0; offset<size; offset+=16) {
        printf("%04X ",a);
        c = ((unsigned char *) area) + offset;
        for(i=0;i<16;i++) {
            if( i == 8 )
                printf("  ");
            printf("%02X",c[i]);
        }
        printf("  ");
        for(i=0;i<16;i++) {
            if( i == 8 )
                printf(" ");
            if( isprint(c[i])) {
                putchar(c[i]);
            } else {
                putchar('.');
            }
        }
        putchar('\n');
        a += 16;
    }
    return;
}
#endif