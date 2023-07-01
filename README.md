Developing for the STM32F746G using GCC on Linux
================================================

>NOTE 1: This shows software under development. They may compile but some of 
them are not tested. Specially, the Xnn projects are not tested and may not even
compile.

> NOTE 2: Only open source *st-link* (st-flash and st-utils) is fully tested.


Introduction
------------

A step-by-step programming guide to a Cortex M7 Microcontroller.

The board is a STM32F746GDISCOVERY board [1]. Its main features are:

* a STM32F746NGH6 micro-controller (up to 216 MHz)
* 1 MByte Flash
* 340 KByte RAM
* a 4.3" LCD color display with 480x272 resolution with touch-screen
* Ethernet interface with a RJ45 connector
* USB OTG High Speed
* USB OTG Full Speed
* 8 MByte SDRAM accessed thru the FMC interface
* 16 MByte QuadSPI Flash memory
* Camera connector
* microSD connector
* on board ST-LINK/v2-1 debugger/programmer

The Gnu ARM Embedded Toolchain [2] is used in all projects.

The host computer is a Linux machine (Linux Mint 20).

The build process is based on Makefiles.


Requisites
----------


> NOTE: It is recommend to upgrade the firmware in the STM32F746G Discovery as described in oard using the software in [8]


These software are needed to build embedded applications on a STM32F7
 micro-controller.

* GNU Arm Toolchain [2]: compiler, linker and other utilities.
* STM32Cube library software [3]. ST HAL library.
* GNU Make [10,11]
* Flash memory programmer (one of them)
  - st-flash (part of stlink) [4]
  - STLink Utility [6]
  - STCubeProgrammer [7]
  - OpenOCD
  
To edit the source files, any editor can be used. It is desirable that it has
support for projects based on makefiles. Microsoft Visual Studio Code [12] 
is a very good tool (Do not confuse with Microsoft Visual Studio IDE, a 
commercial product). It is also possible to use a IDE like Eclipse IDE [13] 
(with CDT extension) or TrueStudio (former Attolic) [14].

Recommended additional software are:

* Git: version control system (already incorporated in Eclipse and Visual 
  Studio Code)
* Doxygen: Documentation generation tool


For debugging, although the GNU ARM Toolchain provides the GNU Debugger
 (GDB) debugger, additional software is needed in order the GDB debugger can
  communicate with the STM32 micro-controller on the board. This is done by 
  a GDB server as shown in the figure below. 

Diagram of the development system for STM32 micro-controllers
-------------------------------------------------------------


    |---- PC ------------------|        |----- Board ----------------------|
    |                          |        |                                  |
    |  |--------|  |--------|  |   USB  | |----------|   |---------------| |                   
    |  | GDB    |  |  GDB   |  |--------|-| Debug uC |---| Target STM32  | |
    |  | inter  |--| Server |  |        | |----------|   |---------------| |
    |  | face   |  |--------|  |        |                                  |
    |  |--------|              |        |----------------------------------|          
    |                          |
    |--------------------------|

Note that there is a serial interface, not shown, between the Debug uC and the
 target STM32 in the figure above. A terminal software can communicate with 
the target STM thru a Virtual COM (CDC=Communication Device Class) made 
available by the Debug uC with support of the operation system in the PC.

To debug an application in the target STM32 from the PC using GDB, a bridge 
provided by a GDB server is needed. The alternatives are:

* open source st-util (part of [4])
* STLink GDB Server (part of [5]. Do not confuse with the software with similar name in [9])
* OpenOCD
* pyOCD 

NOTE: ST-LINK server module (ST-LINK-SERVER) [9] is a tool to share the 
debug interface among tools.

The GDB provided by GNU ARM Toolchain has two interfaces:

* Command Line Interface (CLI). arm-none-eabi-gdb
* Text User interface (TUI) (arm-none-eabi-gdb --tui)

In both cases, the following commands must be entered in order to make it connect 
to the board.

    target extended-remote localhost:3333

or in a short form

    tar ext :3333

Different GDB servers use different port numbers.

Commands to control the device can be found with the monitor help command. 
Generally, the commands below are accepted.

mon halt      (mon hal)
monitor reset (mon res)
monitor init  (mon init)

For Linux, there are software with graphic user interface (GUI) to interfacing 
to a GDB server, like nemiver, ddd. Visual Studio Code can also work as a GUI 
by using the Cortex-Debug extension.



Projects                
--------

|  #   |  Project            |  Description                         | Status |
|------|---------------------|--------------------------------------|--------|
|   01 | Blinker-Simple      | A very simple LED blinker            |   OK   |
|   02 | Blinker-GPIO-HAL    | Simple blinker with a GPIO HAL       |   OK   |
|   03 | Blinker-LED-HAL     | Simple blinker with a LED HAL        |   OK   |
|   04 | Blinker-LED-HAL-V2  | Simple blinker with a better LED HAL |   OK   |
|   05 | Blinker-LED-GPIO-HAL| Simple blinker with a LED/GPIO HAL   |   OK   |
|   06 | Blinker-SysTick     | Simple blinker using SysTick         |   OK   |
|   07 | Blinker-200MHz      | Changing the CPU clock frequency     |   OK   |
|   08 | Button              | Reading button status                |   OK   |
|   09 | UART-Polling        | UART interface with polling          |   OK   |
|   10 | UART-Interrupts     | UART interface with interrupts       |   OK   |
|   11 | Conversions         | Show MCU Info                        |   OK   |
|   12 | Ministdio           | A minimal stdio (printf)             |   OK   |
|   14 | Newlib              | Using newlib (libc replacement)      |   OK   |
|   15 | TimeTriggered-v1    | Pont's time triggered system (old)   |   OK   |
|   16 | TimeTriggered-v2    | Pont's time triggered system (new)   |   OK   |
|   17 | ucos2               | Using uC/OS-II                       |   OK   |
|  X18 | ----                | ----                                 |   -    |
|  X19 | ----                | ----                                 |   -    |
|  X20 | ----                | ----                                 |   -    |
|  X21 | ----                | ----                                 |   -    |
|   22 | ExternalRAM         | Using External (to MCU) RAM          |   -    |
|   23 | Buddy               | Using a Buddy Allocator for RAM      |   -    |
|   24 | LCD                 | Using the LCD (barebones)            |   -    |
|  X25 | I2C-Polling         | ----                                 |   -    |
|  X26 | I2C-Interrupt       | ----                                 |   -    |
|  X27 | I2C-DMA             | ----                                 |   -    |
|  X28 | Touch               | Accessing the touch controller (I2C) |  TBD   |
|  X29 | ----                | ----                                 |   -    |
|  X30 | LCD-LVGL-v1         | Using LVGL library to LCD            |  TBD   |
|  X31 | LCD_ChromArt        | Using ChromArt for LCD (barebones)   |  TBD   |
|  X32 | LCD-LVGL-v2         | Using ChromArt for LVGL/LCD          |  TBD   |
|  X33 | ----                | ----                                 |   -    |
|  X34 | ----                | ----                                 |   -    |
|  X35 | LCD-UGUI            | Using UGUI library for LCD           |  TBD   |
|  X36 | ----                | ----                                 |   -    |
|  X37 | ----                | ----                                 |   -    |
|  X38 | ----                | ----                                 |   -    |
|  X39 | ----                | ----                                 |   -    |
|  X40 | MicroSD             | Accessing uSD card                   |  TBD   |
|  X41 | NORFlash            | Accessing onboard Flash              |  TBD   |
|  X42 | ----                | ----                                 |   -    |
|  X43 | ----                | ----                                 |   -    |
|  X44 | ----                | ----                                 |   -    |
|  X45 | USB-FS_Device       | Board as a USB device                |  TBD   |
|  X46 | ----                | ----                                 |   -    |
|  X47 | ----                | ----                                 |   -    |
|  X48 | ----                | ----                                 |   -    |
|  X49 | ----                | ----                                 |   -    |
|  X50 | Ethernet            | Using the Ethernet interface         |  TBD   |
| ...  | ...                 | ...                                  |  ...   |
| XX60 | Linux               | Using ucLinux                        |  TBD   |


Notes
-----

1. OpenOCD under development

Makefile configuration   
Flash programming (telnet 4444)  


2. UART with interrupt

Single byte buffer not tested.

2. TODO: Teste parameters for compilation

Effects and use of -specs=nosys.specs -specs=nano.specs -specs=rdimon.specs (-lrdimon)

Use minimal newlib: --specs=nano.specs

3. TODO: Test semihosting

Test ITM-SendChar  

monitor arm semihosting enable  


    #include <stdio.h>
    #include <stdlib.h>
    # ….

    extern void initialise_monitor_handles(void);

    int main(void) {
      initialise_monitor_handles();

      printf("Hello !\n");

      HAL_Init();
      SystemClock_Config();

      puts("Check your openocd console.\n");
      printf("This works too\n");
      //……………
    }




Annex A - GDB Servers
---------------------

This annex shows the installation and use of the following GDB servers:

* Open source st-link
* ST STLINK
* pyOCD
### Open source st-link

st-link [4] is a open source toolset written in C to replace the proprietary ST tools. It contains:

* st-flash: writes a binary file to target flash memory
* st-info:  display info about the connected uC.
* st-util: a GDB server
* stlink-gui: a GUI interface to flash memory


#### Install

To install st-link, one can use ONE of these methods.

1. From distribution repositories (generally outdated)
```
    sudo apt install stlink-tools stlink-gui
```
2. From packages built by the st-link developers. They can be downloaded from [17] and installed with the command after purging any installed version
```
    sudo apt purge stlink-tools stlink-gui libstlink1 libstlink-dev
    dpkg -i stlink-1.7.0-1_amd64.deb
```
3. From [4] clone the repository and compile it.
``` 
   git clone https://github.com/stlink-org/stlink.git
   cd stlink
   make clean
   cmake -DCMAKE_INSTALL_PREFIX=/usr
   make install # or checkinstall
```
#### Use

1. In one terminal window start pyOCD
```
   st-util
```
2. In another terminal windows start GDB (Replace PROGRAM with the program name)
```
   arm-none-eabi-gdb PROGRAM.axf
```
3. Enter command to connect to GDB server and the board
   
```
    tar ext :4242
```

4. Reset target, and set a breakpoint in the start of the main routine
```
   mon halt
   mon reset
   b   main
   cont
```

#### Evaluation

It is mature but it has show some problems with GDB not locating function and/or setting breakpoints. It is possible that this is a GDB problem. To be verified!


### OpenOCD

OpenOCD is a Open On-Chip Debugger, that support a lot of interfaces
and micro-controllers, among them, ARM Cortex devices. It can be used as a flash programmer and as a GDB server.

#### Install

OpenOCD can be installed by ONE of the methods below. But mixing methods can lead to strange errors.

1. Using distribution package manager. In Debian Linux and derivatives
   (Ubuntu,Mint), just enter
   ```
   sudo apt install openocd
   ```
2. Install from packages of the xPack repository [18]. Download the compressed file and unpack it
```
    wget https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v0.11.0-1/xpack-openocd-0.11.0-1-linux-x64.tar.gz
    tar -xvf xpack-openocd-0.11.0-1-linux-x64.tar.gz
    mv  xpack-openocd-0.11.0-1 /opt 
```

After this is done, one can either add /opt/xpack-openocd-0.11.0-1/bin/openocd to PATH or make a link in the /usr/bin directory

3. Install from source obtained from [19] and run the commands below.
```
    git clone https://git.code.sf.net/p/openocd/code openocd-code
    cd openocd-code
    ./bootstrap
    ./configure --prefix=/usr
    make
```

When installing in other directory, use the  OPENOCD_SCRIPTS shell variable or the -s parameter to tell where the scripts are.

### Use

OpenOCD needs configuration files in a TCL dialect to instruct it how to operate. For certain known boards, this can be easy

    openocd -f CONFIGURATION_FILE.cfg

Specifically for the STM32F746 Discovery board, there is a configuration file called *stm32f7discovery.cfg* in the *scripts/board* subfolder. 

For other boards, more than one configuration file are needed. One for the adapter and other for the board. 

Commands can be stored in a text file and run using the -c parameter.

    openocd -f ADAPTER.cfg -f BOARD.cfg -c COMMANDFILE

The scripts folder has many configuration files for boards, cpu, interfaces.

    scripts/
        board
        chip
        cpld
        cpu
        fpga
        interface
        target

To start OpenOCD as a GDB server, in a separated window use the command

    openocd -f stm32f7discovery.cfg

Start the GDB in another window and connect it to the OpenOCD server by entering the command

    target extended-remote localhost:3333

To used openocd to write to the micro-controller flash memory, one
can use the following that creates a port 4444 to accept commands.

    openocd -f stm32f7discovery.cfg
    sleep 15
	telnet localhost 4444 

Using a telnet connected to port 4444, one can enter the following command sequence to write the PROGRAM.bin file into the micro-controller flash memory (Replace PROGRAM.bin with the name of the binary file). To send the command, first connect to the OpenOCD, by using the command *telnet localhost 4444*, and then enter the commands below.

	reset halt
	flash probe 0
	flash write_image erase PROGRAM.bin 0x8000000
    reset run
	shutdown

A better way is to write a configuration file that defines a procedure and run it using  * -f configuration.cfg -c "write_to_flash" * parameter. (TO BE VERIFIED!)

    proc write_to_flash {} {
        reset halt
        flash probe 0
        flash write_image erase PROGRAM.bin 0x8000000
        reset run
        shutdown
    }

### ST GDB Server

ST provides the ST-LINK GDB Server as part of the STM32CubeIDE. 

ST provides software to write into the flash memory of the STM32 micro-controllers: ST-LINK Utility (obsolete) [22] and the new STM32CubeProgrammer []

#### Installation

ST-LINK GDB Server is included in the STM32CubeIDE [20]. It is named *ST-LINK_gdbserver* and is located in the subfolder named *com.st.stm32cube.ide.mcu.externaltools.stlink-gdb-server.linux64_1.6.0.202101291314/tools/bin/* of the STM32Cube installation folder (STM32CUBEIDEDIR).

ST-LINK GDB Server uses the STM32CubeProgrammer to flash memory when the *load" command is used. STM32CubeProgrammer can be installed as a standalone software. But it also possible to use the STM32CubeProgrammer installed with the STM32CubeIDE and located in the STM32CUBEIDEDIR/plugin subfolder.

#### Use

The ST-LINK GDB Server is used as any other GDB server. In one window, enter the command

    ST-LINK-gdbserver -d -v -cp STM32CUBEIDEDIR/plugin/com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.linux64_1.6.0.202101291314/tools/bin

The -d parameter means to use the SWD interface and the -v, verbose output. A -e parameter enables to use the extended-remote connection.

And in other window, start GDB and enter the command

    target extended-remote localhost:61234

for persistent mode (recommended) or the command

    target remote localhost:61234

for non-persistent mode.

More information can be found in the ST-LINK GDB Server Manual [20].
### pyOCD 

pyOCD [15] is a ST tool replacement written in Python.

#### Install

PyOCD can be installed by ONE of these methods, but they should not be mixed.

1.  Using package manager. In a Linux Debian system, it can be installed using apt.
```
    sudo apt install python3-pyocd
```
2.  Using python PIP
```
    sudo python3 -mpip install -U pyocd
```
4.  From source code
```
    mkdir PyOCD
    cd PyOCD
    git  clone github.com/pyocd
    cd pyocd
    python3 setup.py install
```
In Ubuntu systems, the udev rules in the udev folder must be installed by copying them to the /etc/udev/rules.d folder and resloading the rules
```
    sudo cp PYOCD/udev/*rules /etc/udev/rules.d
    sudo udevadm control --reload
```

#### Use

1. In one terminal window start pyOCD
```
   pyocd-gdbserver
```
2. In another terminal windows start GDB (Replace PROGRAM with the program name)
```
   arm-none-eabi-gdb PROGRAM.axf
```
3. Enter command to connect to GDB server and the board
```
   tar ext :3333
```
4. Discover which commands are accepted
```
   mon help
```
#### Evaluation

It is unstable. The GDB server must oft be restarted. There is no way to 
reset the board or to detect that the board was reset.

Annex B - Initialization 
------------------------

The initialization procedure used in the SMT32 HAL library do certain tasks, that can be helpful.

### ART Enable

	FLASH->ACR |= FLASH_ACR_ARTEN;
	
#### Flash Prefetch Buffer Enable

	FLASH->ACR |= FLASH_ACR_PRFTEN;

### NVIC Priority Grouping

	NVIC_SetPriorityGrouping

It is possible to divide the 4-bit priority in preemption and subpriority priorities.

### Initialize Tick

	uint32_t lowestprio = (1<<__MVIC_PRIO_BITS)-1;
	SysTick_Config(SystemCoreClock/1000);
	NVIC_SetPriority*SysTick_IRQn,15,0);

Set to the lowest priority

### Disable MPU (Memory Protection Unit)

	__DMB();
	SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;
  	MPU->CTRL = 0;
	
### Enable MPU (Memory Protection Unit)

  	MPU->CTRL = MPU_Control|MPU_CTRL_ENABLE_Msk;
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
	__DSB();
	__ISB();

### Enable caches

	SCB_EnableICache();
	SCB_EnableDCache();
	

### FPU Settings

  	#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    		SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
	#endif

 

References
----------

1 - [Discovery Kit with STM32F746NG MCU](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html)  
2 - [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)  
3 - [STM32Cube MCU Package for STM32F7 series](https://www.st.com/en/embedded-software/stm32cubef7.html)  
4 - [Open source version of the STMicroelectronics STlink Tools](https://github.com/stlink-org/stlink)  
5 - [Integrated Development Environment for STM32](https://www.st.com/en/development-tools/stm32cubeide.html)  
6 - [STM32 ST-LINK utility (replaced by STM32CubeProgrammer)](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html)  
7 - [STM32CubeProgrammer software for all STM32](https://www.st.com/en/development-tools/stm32cubeprog.html)  
8 - [ST-LINK, ST-LINK/V2, ST-LINK/V2-1, STLINK-V3 boards firmware upgrade](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link007.html)  
9 - [ST-LINK server software module ](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-performance-and-debuggers/st-link-server.html)  
10 - [GNU Make](https://www.gnu.org/software/make/)  
11 - [Make for Windows](http://gnuwin32.sourceforge.net/packages/make.htm)  
12 - [Visual Studio Code](https://code.visualstudio.com/)  
13 - [Eclipse IDE](https://www.eclipse.org/eclipseide/)  
14 - [TrueStudio IDE](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-ides/truestudio.html)  
15 - [PyOCD](https://github.com/pyocd/pyOCD)  
16 - [STM32CubeIDE programmer and ST-LINK GDB server on Linux](http://pacinispace.blogspot.com/2020/02/stm32cubeide-st-link-gdb-server-on-linux.html)  
17 - [STLink Releases](https://github.com/stlink-org/stlink/releases)  
18 - [XPack OpenOCD Repository](https://github.com/xpack-dev-tools/openocd-xpack/releases)  
19 - [UM2576 - STM32CubeIDE ST-LINK GDB server](https://www.st.com/resource/en/user_manual/dm00613038-stm32cubeide-stlink-gdb-server-stmicroelectronics.pdf)  
20 - [STM32CubeProg - STM32CubeProgrammer software for all STM32](https://www.st.com/en/development-tools/stm32cubeprog.html)  
21 - [STM32CubeIDE - Integrated Development Environment for STM32](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-ides/stm32cubeide.html)  
22 - [STSW-LINK004 - STM32 ST-LINK utility](https://www.st.com/en/development-tools/stsw-link004.html)  
