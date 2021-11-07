Using the Chrom-Art
=============

Introduction
------------

The STM32F646 has a LCD control interface and an accelerator, called ChromeART (DMA2D).
The board has a 4.3" LCD display with Capacitive Touch Panel (CTP). 

The ChromART is implememented by the DMA2D interface.

It can implement the following function in a very efficient way:


* Filling a part or the whole of a destination image with a specific color
* Copying a part or the whole of a source image into a part or the whole of a destination 
image with a pixel format conversion
* Copying a part or the whole of a source image into a part or the whole of a destination 
image with a pixel format conversion)
* Blending a part and/or two complete source images with a different pixel format and
copying the result into a part or the whole of a destination image with a different color
format.

The ChromART uses the DMA interface. The STM32F746 has two DMA controllers (DMA1 and DMA2), each one
can manage 8 streams and each stream can have up to 8 channels.

Direct Memory Access (DMA)
--------------------------

The Direct Memory Acess (DMA) interface allows peripheral to have direct access to the MCU memory.
And so, a peripheral can write to or read from memory without the CPU.

It can be used by ADC, USART, ETH, and ChromArt interfaces, among others.

The DMA interface can:

* manage transfers with byte, halfword and word width and when source and destination have 
different width, it can pack/upack data accordlingly. To do this, it must be in FIFO mode.
* manage addresses in incrementing and decrementing mode.
* support bursts of 4, 8 or 16 bytes. 
* use a circular buffer.
* generate interrupts (half-transfer, transfer complete, transfer error, FIFO error,
 direct mode error)
 
There are the following interrupts numbers associated with the DMA interface.

| IRQn    |   Name          |     Description                   |
|---------|-----------------|-----------------------------------|
|    11   | DMA1_Stream0    | DMA 1 Stream 0  global interrupt  |
|    12   | DMA1_Stream1    | DMA 1 Stream 1  global interrupt  |
|    13   | DMA1_Stream2    | DMA 1 Stream 2  global interrupt  |
|    14   | DMA1_Stream3    | DMA 1 Stream 3  global interrupt  |
|    15   | DMA1_Stream4    | DMA 1 Stream 4  global interrupt  |
|    16   | DMA1_Stream5    | DMA 1 Stream 5  global interrupt  |
|    17   | DMA1_Stream6    | DMA 1 Stream 6  global interrupt  |
|    56   | DMA2_Stream0    | DMA 2 Stream 0  global interrupt  |
|    57   | DMA2_Stream1    | DMA 2 Stream 1  global interrupt  |
|    58   | DMA2_Stream2    | DMA 2 Stream 2  global interrupt  |
|    59   | DMA2_Stream3    | DMA 2 Stream 3  global interrupt  |
|    60   | DMA2_Stream4    | DMA 2 Stream 4  global interrupt  |
|    68   | DMA2_Stream5    | DMA 2 Stream 5  global interrupt  |
|    69   | DMA2_Stream6    | DMA 2 Stream 6  global interrupt  |
|    70   | DMA2_Stream7    | DMA 2 Stream 7  global interrupt  |


The transfers can use

* FIFO mode
* circular mode
* direct mode

There are three type of transfers:

* register to memory: can be triggered by an interface like UART, ADC, and others
* memory to register: can be triggered by an interface like UART, I2C, etc.
* memory to memory: it is started by setting a bit on the DMA interface

| DMA_SxCR_DIR |   Direction           | Source        | Destination | Count      |
|--------------|-----------------------|---------------|-------------|------------|
|       00     | Peripheral to memory  | DMA_SxPAR     | DMA_SxM0AR  | DMA_SxNDTR |
|       01     | Memory to peripheral  | DMA_SxM0AR    | DMA_SxPAR   | DMA_SxNDTR |
|       10     | Memory to memory      | DMA_SxPAR     | DMA_SxM0AR  | DMA_SxNDTR |

> NOTE: In the STM32F746, only the DMA2 can make memory to memory transfers.

> NOTE: In memory to memory, the circular and direct mode are not allowed.

> NOTE: To disable a stream, clear bin EN in DMA_SxCR and wait until it is cleared.



The Chrom-ART (DMA2D)
---------------------


The Chrom-ART interface, also known as DMA2D, can operate in the following modes:

• Register-to-memory
• Memory-to-memory
• Memory-to-memory with Pixel Format Conversion (PFC)
• Memory-to-memory with Pixel Format Conversion (PFC) and Blending

It incorporates dedicated memory for CLUT (color look-up tables).

It has two ports:

* master: used for memory transfers
* slave: used to program the DMA2D controller.

It can work with two inputs, one for the foreground and other for the background. The output value
is calculated from the formula, where C is a component of the color (R, G or B).

$$ C_{out} = \frac{ C_{FG} \alpha_{FG} + C_{BG} \alpha_{BG} + C_{BG} \alpha_{MULT} }{ \alpha_{OUT} } $$

where  

$$ \alpha_{MULT} = \frac{\alpha_{FG} \alpha_{BG} }{ 255 } $$

$$ \alpha_{OUT} = \alpha_{FG} + \alpha_{BG} - \alpha_{MULT} $$

The main registers are:

* DMA2D_CR: set operating mode and enable, start, suspend, abort transfers
* DMA2D_FGMAR: foreground memory addresss register
* DMA2D_FGOR: foreground offset register
* DMA2D_FGPFCCR: configurate the foreground FIFO
* DMA2D_BGMAR: background memory address register
* DMA2D_BGBOR: background offset register
* DMA2D_BGPFCCR: configurate the background FIFO
* DMA2D_NLR: set number of lines and pixel per lines
* DMA2D_OPFCCR: configure output PFC
* DMA2D_OMAR: output memory address register
* DMA2D_OOR: output offset register
* DMA2D_NLR: set number of lines and pixel per lines

The DMA2D controller support different input formats.

| CM   | Color mode           |
|------|----------------------|
| 0000 | ARGB8888             |
| 0001 | RGB888               |
| 0010 | RGB565               |
| 0011 | ARGB1555             |
| 0100 | ARGB4444             |
| 0101 | L8                   |
| 0110 | AL44                 |
| 0111 | AL88                 |
| 1000 | L4                   |
| 1001 | A8                   |
| 1010 | A4                   |


### Register to memory mode

Using this mode, it is possible to fill a rectangle in the destination area with the
contents of DMA2D_OCOLR. 


### Setup a transfer

Each DMA2D data transfer is composed of up to 4 steps:

1. Data loading from the memory location pointed by the DMA2D_FGMAR register and 
pixel format conversion as defined in DMA2D_FGCR.
2. Data loading from a memory location pointed by the DMA2D_BGMAR register and pixel 
format conversion as defined in DMA2D_BGCR.
3. Blending of all retrieved pixels according to the alpha channels resulting of the PFC
operation on alpha values.
4. Pixel format conversion of the resulting pixels according to the DMA2D_OCR register
and programming of the data to the memory location addressed through the DMA2D_OMAR register.



References
----------
 
1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
2. [STM32F746NG Data sheet](https://www.st.com/resource/en/datasheet/stm32f746ng.pdf)
3. [AN4861 - LCD-TFT display controller (LTDC) on STM32 MCUs](https://www.st.com/resource/en/application_note/dm00287603-lcdtft-display-controller-ltdc-on-stm32-mcus-stmicroelectronics.pdf) 
4. [AN4943 - Using the Chrom-ART AcceleratorTM to refresh an LCD-TFT
display on STM32L496xx/L4A6xx/L4Rxxx/L4Sxxx microcontrollers](https://www.st.com/resource/en/application_note/dm00338361-using-the-chromart-accelerator-to-refresh-an-lcdtft-display-on-stm32l496xxl4a6xxl4rxxxl4sxxx-microcontrollers-stmicroelectronics.pdf)
5. [STM32 Chrom-ART Accelerator](https://www.st.com/resource/en/product_training/STM32F7_System_DMA2D.pdf)



 
 
 
 
