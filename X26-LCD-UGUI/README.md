Using a GUI Library
===================

Introduction
------------

The board has a 4.3" 480x272 graphic display with Capacitive Touch Panel (CTP). 

The display
-----------

It is produced by Rocktech under the code RK043FN48H-CT672B. Its resolution is 480x272 and can
 display 16777216 colors (RGB888 interface). The board uses a customized version of the commercial
 display (It uses a different cable/connector).
 
The display builtin LCD controller is the OTA5180A and the CTP controller is a FT5336GQQ.

> NOTE: There is a HT043c in the schematics!!!!

> NOTE: This controller is considered obsolete and should not be used for new projects!!!!

A LCD driver was developed in other project and it is used here as the basis for a GUI library.

GUI Libraries
-------------

The LCD driver enables drawing onto the display, but it is a low level and to implement a GUI using it, is at least cumbersome and time consuming.

There are some free GUI libraries for embedded systems. Among them

* [UGUI](https://embeddedlightning.com/ugui/)
* [MakiseGUI](https://github.com/SL-RU/MakiseGUI)
* [EasyGUI](https://github.com/MaJerle/EasyGUI)
* [GUILib](https://github.com/Nikolay-Kha/GUILib)
* [lVGL](https://lvgl.io/)
* [emGUI](https://github.com/libEmGUI/emGUI)
* [eGUI](https://github.com/NXPmicro/eGUI)
* [CGUI](http://cgui.sourceforge.net/)
* [uGFX](https://ugfx.io/)
* [GUI-lite](https://github.com/idea4good/GuiLite)
* [MicroMenu-v2](https://github.com/abcminiuser/micromenu-v2)


The UGUI is small (3 files) and simple to use. It uses pure C and has no dependencies.

The UGUI Library
----------------

The UGUI library consists of 3 files:

* ugui.c: implementation
* ugui.h: interface
* ugui_config.h: configuration and compatibility layer

Its demands are very simple:

* a function to set a pixel
* a periodical call to a function called UG_Update.

The *ugui_config.h* permits the definition of

* color mode (RGB888 or RGB565)
* Fonts to be used (4x6 to 32x53 sizes)
* Integer types (it defaults to stdint types)

It makes heavily use of callbacks. Basically every windows has a callback function.

The touch interface is done externally. The function UG_TouchUpdate must be called to inform an event to the library.

It can use accelerated fill area and draw line operations. The corresponding functions must be registered by calling the UG DriverRegister function. The functions prototypes must be as below. 

    /✯ Hardware accelerator for UG DrawLine ✯/
    UG RESULT HW DrawLine ( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c );

    /✯ Hardware accelerator for UG FillFrame ✯/
    UG RESULT HW FillFrame ( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG S16 y2, UG_COLOR c );


References
----------
 
1. [uGUI](https://embeddedlightning.com/ugui/)
2. [LVGL](https://lvgl.io/)
1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
2. [STM32F746NG Data sheet](https://www.st.com/resource/en/datasheet/stm32f746ng.pdf)
3. [AN4861 - LCD-TFT display controller (LTDC) on STM32 MCUs](https://www.st.com/resource/en/application_note/dm00287603-lcdtft-display-controller-ltdc-on-stm32-mcus-stmicroelectronics.pdf) 
4. 
 
