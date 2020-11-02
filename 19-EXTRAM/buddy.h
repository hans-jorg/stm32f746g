#ifndef BUDDY_H
#define BUDDY_H
/**
 *  @file   buddy.h
 *
 *  @author Hans
 *  @date   27/10/2020
 */


/**
 *  @brief  Size definition
*/
///@{
/// Total size of area to be managed. Must be a power of 2
#ifndef BUDDYTOTALSIZE
#define BUDDYTOTALSIZE   16384
#endif
/// Minimal size of a block
#ifndef BUDDYMINSIZE
#define BUDDYMINSIZE     1024
#endif
/// Address of area to be managed
#ifndef BUDDYBASE
#define BUDDYBASE        0x1000000
#endif
///@}

void  buddy_init(void);
void *buddy_alloc(unsigned size);
void  buddy_free(void *addr);

#ifdef DEBUG
void buddy_printmap(void);
void buddy_printaddresses(void);
#endif
#endif

