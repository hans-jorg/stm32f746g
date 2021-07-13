#ifndef DEBUGMESSAGES_H
#define DEBUGMESSAGES_H
/**
 * @file    debugmessages.h
 *
 * @note    This version uses the standard library vprintf function.
 *
 * @note    Another version using a macro with variable number of arguments was
 *          tried. But such macros must have at least two arguments. Dismissed!!!!
 */

#include <stdio.h>

///@{
int verbose  __attribute__((weak)) = 0;


static void message(char *msg,...) {

    va_list args;

    va_start(args,msg);

    if( verbose ) {
        vprintf(msg,args);
    }
    va_end(args);
}

#define MESSAGE(TEXT)  \
        do {\
            if( verbose ) message(TEXT); \
        } while(0)
#if __STDC_VERSION__ >= 199901L
#define MESSAGEV(TEXT,...) \
        do { \
            if ( verbose ) message(TEXT, __VA_ARGS__); \
        } while(0)
#else
#define MESSAGEV
#endif
///@}

#endif   // DEBUGMESSAGES_H