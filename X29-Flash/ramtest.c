/**
 * @file    ramtest.c
 *
 * @brief   <Short description>
 *
 * @note    Stack must be in a different memory unit
 *
 * @note

 *
 * @author  Hans
 *
 * @version 1.0
 *
 * @date    XX/XX/2020
 */

 #include <stdio.h>
 #include <stdint.h>


void
ramtest( void *startaddress, unsigned size) {
intptr_t start = (intptr_t) startaddress;
intptr_t end   = (intptr_t) ((uint8_t *) startaddress+size);


}
