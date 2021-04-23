#ifndef BUDDY_H
#define BUDDY_H
/**
 *  @file   buddy.h
 *
 *  @author Hans
 *  @date   27/10/2020
 */

#include "sdram.h"



int   Buddy_Init(char *addr, long size, long minsize);
void *Buddy_Alloc(unsigned size);
void  Buddy_Free(void *addr);

#ifdef DEBUG
void  Buddy_PrintMap(void);
void  Buddy_PrintAddresses(void);
#endif
#endif

