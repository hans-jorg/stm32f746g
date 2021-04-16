Info using conversion routines
==============================

Introduction
------------

It is very common in embedded systems the need to convert information from a string to an integer or vice-versa.

This project uses the following functions:

void IntToString(int v, char *s);  
void UnsignedToString(unsigned x, char *s);  
void IntToHexString(unsigned, char *s);  

To test this functions, a set of information about the processor is printed.

### MCU Registers

Address     |  size |   Description
------------|-------|--------------------------------------
0x1FF0F442  |   16  |   Flash memory size in KB
0x1FF0F420  |   96  |   CPU Id (XY pos, lot #, wafer #)
0xE0042000  |   32  |   Model id

### Clock frequencies

Clock signal       |  Description
-------------------|----------------------------
System Core Clock  | Clock used by core
System Clock       | Clock before prescaler
AHB Clock          | The same as System Core Clock
APB1               | Peripheral Clock 1
APB2               | Peripheral Clock 2

### Memory map

Position    |
------------|
Flash start |  
Flash end   |  
RAM start   |
RAM end     |
RAM used    | 
Flash used  | 

### Sections

Position    |
------------|
Code start  |
Code end    |
Data start  |
Data end    |
BSS start   |
BSS end     |


