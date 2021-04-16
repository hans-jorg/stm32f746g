
Info using conversion routines
==============================

Introduction
------------

It is very common in embedded systems the need to print information.

The C standard I/O already has some functions, but link the full libc can be an overkill.

A small subset of functions, similar to the ones in the standard library but very simplified is used in this project

int printf(const char *fmt, ...);  
int puts(const char *s);  
int fputs(const char *s, void *ignored);  
char *fgets(char *s, int n, void *ignored);  

The *printf* only print integer values and does not used allow size, alignment or fill specifications. For example, there is only *%d* and not *%10d*.

There is a simplified *fgets* for input. It only allows line buffering.

The info printed is identical to ones in last project, but it loos more like a standard C code.