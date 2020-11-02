#ifndef BITVECTOR_H
#define BITVECTOR_H
/**
 *  @file   bitvector.h
 *
 * @author  Hans
 * @date    21/10/2020
 */


#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 *  @brief  These symbols define the data type used to store the bit vector,
 *          its size and how to get the index part (which element) and bit
 *          part (which bit).
 *
 *  @note   When changing theses symbols, the format specifier in bv_dump
 *          must be adjusted.
 */
///@{
/// Type used to store the bit vector
#define BV_TYPE     uint32_t
/// Number of bits in the type BV_TYPE
#define BV_BITS     (32)
/// Constante One according type. When using long. use 1UL
#define BV_ONE      (1U)
/// This is a divide by BV_BITS using shifts
#define BV_SHIFT    5
/// The rest of division by BV_BITS
#define BV_BITMASK  0x1F
///@}

/**
 *  @brief  data type for parameters
 */
typedef BV_TYPE *bv_type;

/**
 *  @brief  Size of vector in BV_TYPE
 */
#define BV_SIZE(N) (((N)+BV_BITS-1)/BV_BITS)
/**
 *  @brief  Macros used in bit manipulation
 */
///@{
#ifdef BV_ENABLEMACROS
/// Returns the element where the bit is
#define BV_INDEX(BIT)       ((BIT)>>BV_SHIFT)
/// Returns the bit position in the element
#define BV_BIT(BIT)         ((BIT)&BV_BITMASK)
// Returns a mask with a bit set on position BIT and all others cleared
#define BV_MASK(BIT)        (BV_ONE<<BV_BIT(BIT))
/// Set bit BIT in bit vector X
#define BV_SET(X,BIT)       X[BV_INDEX(BIT)] |= (BV_MASK(BIT))
/// Clear bit BIT in bit vector X
#define BV_CLEAR(X,BIT)     X[BV_INDEX(BIT)] &= ~(BV_MASK(BIT))
/// Test bit BIT in bit vector X, return a non zero value if it is set
#define BV_TEST(X,BIT)     (X[BV_INDEX(BIT])]&(BV_MASK(BIT)))

#endif
///@}

/**
 *  @brief  bv_index
 *
 *  @note   returns the index of the element where the bit is
 */
static inline int
bv_index(int bit) {
    return bit>>BV_SHIFT;
}
/**
 *  @brief  bv_bit
 *
 *  @note   returns the bit position of BIT inside the element where the bit is
 */
static inline int
bv_bit(int bit) {
    return bit&BV_BITMASK;
}
/**
 *  @brief  bv_mask
 *
 *  @note   returns a BV_TYPE bit mask where the bit corresponding to BIT is set
 */
static inline BV_TYPE
bv_mask(int bit) {
    return BV_ONE<<bv_bit(bit);
}
/**
 *  @brief  bv_set
 *
 *  @note   set bit BIT in bit vector v
 */
static inline void
bv_set(bv_type v, int bit) {
    v[bv_index(bit)] |= bv_mask(bit);
}
/**
 *  @brief  bv_clear
 *
 *  @note   clear bit BIT in bit vector v
 */
static inline void
bv_clear(bv_type v, int bit) {
    v[bv_index(bit)] &= ~bv_mask(bit);
}
/**
 *  @brief  bv_test
 *
 *  @note   returns a non zero value if bit BIT in bit vector V is set
 */
static inline BV_TYPE
bv_test(bv_type v, int bit) {
int i = bv_index(bit);
    return v[i] & bv_mask(bit);
}


/**
 *  @brief  bv_setall
 *
 *  @note   set all bits in bit vector
 */
static inline void
bv_setall(bv_type v, int size) {
int i;
    for(i=0;i<BV_SIZE(size);i++) {
        v[i] = (unsigned) -1;
    }
}


/**
 *  @brief  bv_clearall
 *
 *  @note   clear all bits in bit vector
 */
static inline void
bv_clearall(bv_type v, int size) {
int i;
    for(i=0;i<BV_SIZE(size);i++) {
        v[i] = 0;
    }
}


/**
 *  @brief  bv_toggleall
 *
 *  @note   toggle all bits in bit vector
 */
static inline void
bv_toggleall(bv_type v, int size) {
int i;
    for(i=0;i<BV_SIZE(size);i++) {
        v[i] |= (unsigned) -1;
    }
}


#ifdef DEBUG
#ifdef BV_ENABLEMACROS
/// Call bv_dump. Complex instructions are generally not inlined
#define BV_DUMP(X,SIZE)  bv_dump((X),(SIZE))
#endif


/**
 *  @brief  bv_index
 *
 *  @note   returns the index of the element where the bit is
 */

static void bv_dump( bv_type x, int size) {
int i;

    for(i=0;i<BV_SIZE(size);i++) {
        printf("%03d: %08X\n",i,(unsigned) x[i]);
    }
}
#endif


/**
 *  @brief  Macro to create a bit vector area
 */

#define BV_DECLARE(X,SIZE) \
        BV_TYPE X[BV_SIZE(SIZE)]
#endif
