#ifndef CONV_H
#define CONV_H
/**
 * @file     conversions.h
 * @brief    Routines for conversions of int to/from strings
 *
 * @version  V1.00
 * @date     25/3/2016
 *
 * @note
 *
 **/

void IntToString(int v, char *s);
void UnsignedToString(unsigned x, char *s);
void IntToHexString(unsigned, char *s);

#endif // CONV_H
