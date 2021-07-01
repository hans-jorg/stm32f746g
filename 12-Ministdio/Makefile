##
# Makefile for ARM Cortex cross-compiling
#
#
#  @file     Makefile
#  @brief    General Makefile for Cortex-M Processors
#  @version  V1.2
#  @date     16/04/2021
#
#  @note     CMSIS library used
#
#  @note     options
#   @param build       generate binary file
#   @param flash       transfer binary file to target (aliases=burn|deploy)
#   @param force-flash recover board when flash can not be written
#   @param disassembly generate assembly listing in a .dump file
#   @param size        list size of executable sections
#   @param nm          list symbols of executable
#   @param edit        open source files in a editor
#   @param gdbserver   start debug daemon (Start before debug session)
#   @param debug       enter a debug session (one of below)
#   @param  gdb        enter a debug session using gdb
#   @param  ddd        enter a debug session using ddd (GUI)
#   @param  nemiver    enter a debug session using nemiver (GUI)
#   @param  tui        enter a debug session using gdb in text UI
#   @param doxygen     generate doc files (alias=docs)
#   @param clean       clean all generated files
#   @param help        print options
#
#

###############################################################################
# Main parameters                                                             #
###############################################################################

#
# Program name
#
PROGNAME=ministdio

#
# Defines the part type that this project uses.
#
PART=STM32F746
# Used to define part in header file
PARTCLASS=STM32F7xx
# Used to find correct CMSIS include file
PARTCLASSCMSIS=STM32F7xx

#
# Suppress warnings
# Comment out to have verbose output
#MAKEFLAGS+= --silent
.SILENT:

#
# Main target
#
#default: help
default: build

#
# Compatibility Windows/Linux
#
ifeq (${OS},Windows_NT)
HOSTOS :=Windows
else
HOSTOS :=${shell uname -s}
endif

#
# Include debug information
#
DEBUG=y

#
# Include path for CMSIS headers
#

# CMSIS Dir
CMSISDIR=../../STM32CubeF7/Drivers/CMSIS

CMSISDEVINCDIR=${CMSISDIR}/Device/ST/${PARTCLASSCMSIS}/Include
CMSISINCDIR=${CMSISDIR}/Include
INCLUDEPATH=${CMSISDEVINCDIR} ${CMSISINCDIR}

#
# Source files
#
SRCFILES=${wildcard *.c}
#SRCFILES= main.c

#
# Flags specific for project (C, ASM and LD)
#
PROJCFLAGS=-I.
PROJAFLAGS=
PROJLDFLAGS=

#
# Include the common make definitions.
#
PREFIX:=arm-none-eabi

#
# Processor configurations
#
# STM32F746 does not have hardware support for double precision.
# It uses a software library to do all double precision calculation
#

# Set the compiler CPU/FPU options.
#
# Option 1: No floating point (TESTED)
#
CPUFLAGS= -mthumb  -mcpu=cortex-m7+nofp
#
# Option 2: Floating point using hardware but using softfp ABI
#           STM32F746 only has hardware support for single precision FP
#
#CPUFLAGS= -mthumb  -mcpu=cortex-m7+nofp.dp
#FPUFLAGS= -mfloat-abi=softfp -mfpu=fpv5-sp-d16

#
# Option 3: Floating point using hardware but using hard ABI
#           STM32F746 only has hardware support for single precision FP
#
#CPUFLAGS= -mthumb  -mcpu=cortex-m7+nofp.dp
#FPUFLAGS= -mfloat-abi=hard  -mfpu=fpv5-sp-d16

#
# Specs script (modification of compiler and linker flags}
#
# This parameter is only recognized by gcc.
# Linking must be done by a gcc call instead of ld
#
# Alternatives are:
#   nosys.specs:    no libc or libm
#   nano.specs:     minimal libc (newlib-nano)
#   rdimon.specs:   semihosting (serial interface thru debug lines)
#   rdpmon.specs:   RDP
#   redboot.specs:
#   picolibc.specs:
#
SPECFLAGS= --specs=nano.specs

#
# Folder for object files
#
OBJDIR=gcc


#
# Use sections to optimize code generation.
# Functions and data are put in separated sections.
# The linker can drop a (function or data) section
# if there is no reference to i
#

ASECTIONS=  -ffunction-sections                     \
            -fdata-sections                         \


CSECTIONS=  -ffunction-sections                     \
            -fdata-sections                         \

LSECTIONS=  -gc-sections


#
# C Error and Warning Messages Flags
#    -Wall -std=c11 -pedantic
#
CERRORFLAGS=                                        \
            -std=c11                                \
            -pedantic                               \


#
# Terminal application (used to open new windows in debug)
#
#TERMAPP=xterm
TERMAPP=gnome-terminal

#
# Serial terminal communication
#
TTYTERM=/dev/ttyACM0
TTYBAUD=9600


#
# Serial terminal emulator
#
# Use one of configuration below
# cu
#TTYPROG=cu
#TTYPARMS=-l ${TTYTERM} -s ${TTYBAUD}
# screen
#TTYPROG=screen
#TTYPARMS= ${TTYTERM} ${TTYBAUD}
# minicom
#TTYPROG=minicom
#TTYPARMS=-D ${TTYTERM} -b ${TTYBAUD}
# putty
#TTYPROG=putty
# tip
#TTYPROG=tip
#TTYPARMS=-${TTYBAUD} ${TTYTERM}
# picocom
TTYPROG=picocom
TTYPARMS= -b ${TTYBAUD}  ${TTYTERM}

#
# Editor to be used
#
EDITOR=gedit

#
# The command to flash the device
#
# There are five ways to write to flash
#    stflash:   This uses the st-flash utility from Open Source ST-LINK,
#               that can be found in https://github.com/stlink-org/stlink
#               and in many linux repositories.
#               NOTE: Upgrading the board firmware can break the st-flash.
#               This can be solved by installing a new version of the
#               utility.
#    openocd:   The OpenOCD (Open On-Chip Debugging) project has a server
#               that can work as a GDB Server and as a flasher. To write a
#               binary file, a command sequence must be entered using a
#               telnet connection to port 4444. It can be found on
#               https://www.openocd.org.
#    copy:      The board appears as a MSD (Mass Storage Device), i.e., a
#               memory like a pen driver. What is moved to this device
#               is written to the flash. The board appears always with the
#               same name.
#    cube:      ST delivers a tool called to write into flash memory
#               of STM32 devices. There is a CLI version that can be
#               used in a Makefile (not tested yet). It can be found on
#               https://www.st.com/en/development-tools/stm32cubeprog.html
#    stlink:    Windows only. It uses an old utility from ST. It can be found on
#               https://www.st.com/en/development-tools/stsw-link004.html.
#               Not tested yet.
#
flash: flash-stflash
#flash: flash-openocd
#flash: flash-cube
#flash: flash-copy
ifeq (${HOSTOS},Windows)
#flash: flash-stlink
endif

#
# Default debugger
#
# There are many alternatives
#   gdb:        A command line interface (CLI) to GDB
#   tui:        A curses interface to GDB
#   gdb:        Another curses interface to GDB
#   ddd:        A X-Windows based GUI interface to GDB
#   nemiver:    A GTK+ GUI based interface to GDB
#
#
debug: gdb


#
# GDB Server
#
# There are three ways to start a GDB server:
#    stutil:    It uses the st-util utility, that is part of the Open Source
#               ST-LINK. The default port is 4242. It can be found at
#               https://github.com/stlink-org/stlink
#    openocd:   The OpenOCD (Open On-Chip Debugging) project has a server
#               that can work as a GDB Server and as a flasher. To use it as
#               a GDB server, GDB (or a GDB fronted) must connect to port 3333.
#               It can be found at https://www.openocd.org.
#    stlink:    There is a GDB Server embedded in the STM32CubeIDE. It can be
#               used as a standalone apllication. The port used is 61234.
#               STM32CubeIDE can be found at
#               https://www.st.com/en/development-tools/stm32cubeide.html.
#               In Ubuntu systems, the ST software only works correctly
#               when started in its folder.
#
#
gdbserver:gdbserver-stutil
#gdbserver=gdbserver-openocd
#gdbserver=gdbserver-cube


#
# Parameters for Flash and GDB Server software
#

#
# Flash parameters using cp do STM32F746 MSD
#
# Status: tested OK
DEVICENAME=DIS_F746NG
DEVICEMOUNTPOINT=/media/${USER}
COPY=cp

# Flash parameters for open source stlink (st-flash and st-util)
#
# Status: tested OK but it does not work on VS Code
STFLASH=st-flash
STUTIL=st-util
STFLASHCMD=write
STFLASHADDR=0x08000000
STGDBPORT=4242

#
# Configuration for STM32CubeIDE GDB Server
# Note: STM32CubeProgrammer must be installed
#
# Status: Not tested
STCUBEGDBSERVER=stlink-gdbserver
STCUBEPROGRAMMER=STM32CubeProgrammer
CUBEGDBPORT=61234

#
# Parameters for OpenOCD
#
# Status: tested OK
OPENOCD=openocd
OPENOCDDIR=/usr/share/openocd
OPENOCDBOARD=${OPENOCDDIR}/scripts/board/stm32f7discovery.cfg
OPENOCDGDBPORT=3333
OPENOCDTELNETPORT=4444
OPENOCDFLASHSCRIPT=${OBJDIR}/flash.ocd

#
# Additional libraries like RTOS
#
#

EXTSRCFILES=
EXTOBJFILES=
EXTINCLUDEPATH=
EXTCFLAGS=
EXTAFLAGS=
EXTLDFLAGS=

###############################################################################
# Commands                                                                    #
###############################################################################

#
# The command for calling the compiler.
#
CC=${PREFIX}-gcc

#
# The command for calling the library archiver.
#
AR=${PREFIX}-ar

#
# The command for calling the linker.
#
LD=${PREFIX}-ld

#
# Tool to generate documentation
#
DOXYGEN=doxygen

#
# The command for extracting images from the linked executables.
#
OBJCOPY=${PREFIX}-objcopy

#
# The command for disassembly
#
OBJDUMP=${PREFIX}-objdump

#
# The command for listing size of code
#
OBJSIZE=${PREFIX}-size

#
# The command for listing symbol table
#
OBJNM=${PREFIX}-nm

#
# Debuggers
#

## GDB with and without TUI
GDB=${PREFIX}-gdb

## nemiver
NEMIVER=nemiver
NEMIVERFLAGS=

## ddd
DDD=ddd
DDDFLAGS=

## cdbg
CDBG=cdbg
CDBGFLAGS=

## kdbg
KDBG=kdbg
KDBGFLAGS=


###############################################################################
# Commands parameters                                                         #
###############################################################################

#
# Flags for GDB
#
GDBINIT=${OBJDIR}/gdbinit
GDBFLAGS=-x ${GDBINIT} -n


#
# Flags for disassembler
#
ODFLAGS=-S -D

#
# Configuration file for Doxygen
#
DOXYGENCFG=Doxyfile

#
# Tell the compiler to include debugging information if the DEBUG environment
# variable is set.
#
ifeq (${DEBUG},y)
DEBUGCFLAGS=-g -DDEBUG
DEBUGLDFLAGS=-O0 -g
else
DEBUGCFLAGS=
DEBUGLDFLAGS=-Os
endif


###############################################################################
# Generally it is not needed to modify the lines below                        #
###############################################################################

###############################################################################
# Compilation parameters                                                      #
###############################################################################

#
# Get the location of libgcc.a from the GCC front-end.
#
LIBGCC:=${shell ${CC} ${CPUFLAGS} ${FPUFLAGS} -print-libgcc-file-name}

#
# Get the location of libc.a from the GCC front-end.
#
LIBC:=${shell ${CC} ${CPUFLAGS} ${FPUFLAGS} -print-file-name=libc.a}

#
# Get the location of libm.a from the GCC front-end.
#
LIBM:=${shell ${CC} ${CPUFLAGS} ${FPUFLAGS} -print-file-name=libm.a}

#
# Object files
#
OBJFILES=${addprefix ${OBJDIR}/,${SRCFILES:.c=.o}}

#
#
# The flags passed to the assembler.
#
AFLAGS= ${CPUFLAGS}                             \
	    ${FPUFLAGS}                             \
	    ${PROJAFLAGS}                           \
	    ${EXTAFLAGS}                            \
	    ${addprefix -I ,${INCLUDEPATH}}         \
	    ${addprefix -I ,${EXTINCLUDEPATH}}      \
	    ${ASECTIONS}                            \


#
# The flags passed to the compiler.
#
CFLAGS= ${CPUFLAGS}                             \
	    ${FPUFLAGS}                             \
	    ${DEBUGCFLAGS}                          \
	    ${PROJCFLAGS}                           \
	    ${EXTCFLAGS}                            \
	    ${addprefix -I ,${INCLUDEPATH}}         \
	    ${addprefix -I ,${EXTINCLUDEPATH}}      \
	    -D${PARTCLASS}                          \
	    -DPART_${PART}                          \
	    ${CSECTIONS}                            \
	    ${CERRORFLAGS}                          \


#
# The flags passed to the linker.
#
LDFLAGS=                                        \
            ${LSECTIONS}                        \
            ${MAPFLAGS}                         \
            ${DEBUGFLAGS}                       \

#
# linker flags for libraries
#     -nostdlib
#     -nodefaultlibs
LIBFLAGS= -nolibc -nodefaultlibs  -nostdlib


#
# libraries linked
#
# Thery are modified by the specs files

#     -lm -lc -lgcc
LIBS=
#
#

#
# Flags needed to generate dependency information
#
DEPFLAGS=-MT $@  -MMD -MP -MF ${OBJDIR}/$*.d

#
# Linker script
#
#LINKERSCRIPT=${PROGNAME}.ld
LINKERSCRIPT=${shell echo ${PART}| tr A-Z a-z}.ld

#
# Entry Point
#
ENTRY=Reset_Handler

#
# Cflow parameters
#
CFLOWFLAGS=-l  -b --omit-arguments

###############################################################################
# RULES                                                                       #
###############################################################################

COMMA=,
#
# The rule for building the object file from each C source file.
#
${OBJDIR}/%.o: %.c
	@echo "  Compiling           ${notdir ${<}}";
	${CC} -c ${SPECFLAGS} ${CFLAGS} ${CPUFLAGS} ${FPUFLAGS} ${DEPFLAGS} -o ${@} ${<}

#
# The rule for building the object file from each assembly source file.
#
${OBJDIR}/%.o: %.S
	@echo "  Assembling          ${notdir ${<}}";
	${CC} -c  ${SPECFLAGS} ${AFLAGS} ${CPUFLAGS} ${FPUFLAGS} -o ${@} -c ${<}

#
# The rule for creating an object library.
#
${OBJDIR}/%.a:
	@echo "  Archiving           ${@}";
	${AR} -cr ${@} ${^}


###############################################################################
# TARGETS                                                                     #
###############################################################################

#
# help menu
#
help: usage
usage:
	@echo "Options are:"
	@echo "build:       generate binary file"
	@echo "flash:       transfer binary file to target (aliases=burn|deploy)"
	@echo "force-flash: recover board when flash can not be written"
	@echo "disassembly: generate assembly listing in a .dump file"
	@echo "size:        list size of executable sections"
	@echo "nm:          list symbols of executable"
	@echo "edit:        open source files in a editor"
	@echo "gdbserver:   start debug daemon (Start before debug session)"
	@echo "debug:       enter a debug session (one of below)"
	@echo " gdb:        enter a debug session using gdb"
	@echo " ddd:        enter a debug session using ddd (GUI)"
	@echo " nemiver:    enter a debug session using nemiver (GUI)"
	@echo " tui:        enter a debug session using gdb in text UI"
	@echo "doxygen:     generate doc files (alias=docs)"
	@echo "term:        starts a new window with a terminal connected to board"
	@echo "clean:       clean all generated files"
	@echo "help:        print options (default)"

#
# The default rule, which causes the ${PROGNAME} example to be built.
#
build: ${OBJDIR} ${OBJDIR}/${PROGNAME}.bin ${OBJDIR}/${PART}.svd
	echo "Done."

#
# The rule to clean out all the build products.
#
clean:
	rm -rf ${OBJDIR} ${wildcard *~} html latex docs  null.* && echo "Done."

#
# Rules for building binary file from the ${PROGNAME}.axf executable file.
#
${OBJDIR}/${PROGNAME}.bin: ${OBJDIR} ${OBJDIR}/${PROGNAME}.axf
	@echo "  Generating binary ${@}"
	${OBJCOPY} -O binary  ${OBJDIR}/${PROGNAME}.axf ${@}

#
# The rule for linking the application.
#
${OBJDIR}/${PROGNAME}.axf:  ${OBJFILES} ${EXTOBJFILES}
	@echo "  Linking             ${@} ";
	${CC}   -Wl,-T '${LINKERSCRIPT}'                                    \
	        -nostartfiles                                               \
	        --entry '${ENTRY}'                                          \
	        ${DEBUGLDFLAGS}                                             \
	        ${SPECFLAGS}                                                \
	        ${CPUFLAGS}                                                 \
	        ${FPUFLAGS}                                                 \
	        ${LIBFLAGS}                                                 \
	        -Wl,--print-memory-usage                                    \
	        ${addprefix -Wl${COMMA},${LDFLAGS} }                        \
	        ${addprefix -Wl${COMMA},${PROJLDFLAGS} }                    \
	        ${addprefix -Wl${COMMA},${EXTLDFLAGS} }                     \
	        -o ${@} ${OBJFILES}  ${EXTOBJFILES}                         \
	        '${LIBM}' '${LIBC}' '${LIBGCC}'

#
# Rules for the transfer binary to board

#
# Alternate commands (synonyms for flash)
#
burn: flash
deploy: flash


# Flash using copy
flash-copy: ${OBJDIR}/${PROGNAME}.bin
	@echo "  Flashing ${PROGNAME}.bin using copy"
	${COPY}  $^   ${DEVICEMOUNTPOINT}/${DEVICENAME}

# Flash using st-flash
flash-stflash: ${OBJDIR}/${PROGNAME}.bin
	@echo "  Flashing ${PROGNAME}.bin using st-flash"
	${STFLASH} ${STFLASHCMD} $^ ${STFLASHADDR}

# Flash using OpenOCD
flash-openocd: ${OBJDIR}/${PROGNAME}.bin ${OPENOCDFLASHSCRIPT}
	@echo "  Flashing ${PROGNAME}.bin using openocd"
	${OPENOCD} -f ${OPENOCDBOARD}
	sleep 15
	telnet localhost 4444 < ${OPENOCDFLASHSCRIPT}

${OPENOCDFLASHSCRIPT}:
	echo "reset halt" > ${OPENOCDFLASHSCRIPT}
	echo "flash probe 0" >> ${OPENOCDFLASHSCRIPT}
	echo "flash write_image erase ${OBJDIR}/${PROGNAME}.bin 0x8000000" >> \
	    ${OPENOCDFLASHSCRIPT}
	echo "reset run" >> ${OPENOCDFLASHSCRIPT}
	echo "shutdown" >> ${OPENOCDFLASHSCRIPT}

# Flash using st-link
flash-stlink: ${OBJDIR}/${PROGNAME}.bin
	echo "Not implemented yet"
	false

#
# Force write to flash memory. Useful in case of recurring write errors
#
force-flash: ${OBJDIR}/${PROGNAME}.bin
	echo "Press RESET during write"
	sleep 50
	sudo ${FLASHER} --reset write  $^  ${STFLASHADDR}

#
# Debug command
#
gdb: ${OBJDIR}/${PROGNAME}.bin ${GDBINIT}
	${GDB} ${GDBFLAGS} ${OBJDIR}/${PROGNAME}.axf

#
# iDebug command with text UI
#
tui: ${OBJDIR}/${PROGNAME}.bin ${GDBINIT}
	${GDB} -tui ${GDBFLAGS} ${OBJDIR}/${PROGNAME}.axf

#
# iDebug command with text UI
#
cgdb: ${OBJDIR}/${PROGNAME}.bin ${GDBINIT}
	cgdb -d `which ${GDB}`  -x ${OBJDIR}/gdbinit ${OBJDIR}/${PROGNAME}.axf


#
# Debug using GUI
#
ddd: ${OBJDIR}/${PROGNAME}.bin ${GDBINIT}
	ddd --debugger "${GDB} ${GDBFLAGS}" ${OBJDIR}/${PROGNAME}.axf

#
# Debug using kdbg GUI
#
#kdbg: ${OBJDIR}/${PROGNAME}.bin ${GDBINIT}
#	kdbg  -r localhost:${GDBPORT} ${OBJDIR}/${PROGNAME}.axf

#
# Debug using nemiver GUI
#
nemiver: ${OBJDIR}/${PROGNAME}.bin ${GDBINIT}
	nemiver  --remote=localhost:${GDBPORT}  \
	         --gdb-binary=`which ${GDB}`     ${OBJDIR}/${PROGNAME}.axf

#
# Start debug demon
#

# GDB Server using Open Source ST-LINK
gdbserver-stutil: gdbinit-stutil
	#if [ X"`pidof ${STUTIL}`" != X ]; then kill `pidof ${STUTIL}`; fi
	${TERMAPP} -- ${STUTIL} -p ${STGDBPORT}

# GDB Server using OpenOCD
gdbserver-openocd: gdbinit-openocd
	${TERMAPP} -- ${OPENOCD} -f ${OPENOCDBOARD}

#  GDB Server using STM32CubeIDE GDB Server
gdbserver-cube: gdbinit-cube
	${TERMAPP} -- ${CUBEGDBSERVER}

#
# Debugger initialization scripts
#
gdbinit-stutil: FORCE
	echo "# Run this script using gdb source command" > ${GDBINIT}
	echo "target extended-remote localhost:${STGDBPORT}" >> ${GDBINIT}
	echo "break main" >> ${GDBINIT}
	echo "monitor jtag_reset" >> ${GDBINIT}
	echo "monitor halt" >> ${GDBINIT}

gdbinit-openocd: FORCE
	echo "# Run this script using gdb source command" > ${GDBINIT}
	echo "target extended-remote localhost:${OPENOCDGDBPORT}" >> ${GDBINIT}
	echo "break main" >> ${GDBINIT}
	echo "continue" >> ${GDBINIT}

gdbinit-cube: FORCE
	echo "# Run this script using gdb source command" > ${GDBINIT}
	echo "target extended-remote localhost:${CUBEGDBPORT}" >> ${GDBINIT}
	echo "break main" >> ${GDBINIT}
	echo "continue" >> ${GDBINIT}

#
# Disassembling
#
disassembly:${OBJDIR}/${PROGNAME}.dump
dump: disassembly
${OBJDIR}/${PROGNAME}.dump: ${OBJDIR}/${PROGNAME}.axf
	@echo "  Disassembling       ${^} and storing in ${OBJDIR}/${PROGNAME}.dump"
	${OBJDUMP} ${ODFLAGS} $^ > ${OBJDIR}/${PROGNAME}.dump

#
# List size
#
size: ${OBJDIR}/${PROGNAME}.axf
	${OBJSIZE} $^

#
# List symbols
#
nm: ${OBJDIR}/${PROGNAME}.axf
	${OBJNM} $^

#
# The rule to create the target directory.
#
${OBJDIR}:
	mkdir -p ${OBJDIR}

#
# SVD File (used by VS Code)
#
${OBJDIR}/${PART}.svd: ${OBJDIR}
	echo "  Copying ${PART}.svd file to build folder"
	cp ../${PART}.svd ${OBJDIR}

#
# Open files in editor windows
#
edit:
	${EDITOR} Makefile *.c *.h *.ld &


#
# Generate documentation using doxygen
#
docs: doxygen
doxygen: ${DOXYGENCFG}
	${DOXYGEN} ${DOXYGENCFG}
	echo Done.

#
# Generate Doxygen Config
#
SEDSCRIPT=dox.sed
${DOXYGENCFG}:
	${DOXYGEN} -g ${DOXYGENCFG}
	echo /^PROJECT_NAME/cPROJECT_NAME           = \"${PROGNAME}\" > ${SEDSCRIPT}
	echo /^FULL_PATH_NAMES/cFULL_PATH_NAMES     = NO >> ${SEDSCRIPT}
	echo /^OPTIMIZE_OUTPUT_FOR_C/cOPTIMIZE_OUTPUT_FOR_C    = YES >> ${SEDSCRIPT}
	echo /^DISTRIBUTE_GROUP_DOC/cDISTRIBUTE_GROUP_DOC    = YES >> ${SEDSCRIPT}
	echo /^EXTRACT_STATIC/cEXTRACT_STATIC    = YES >> ${SEDSCRIPT}
	echo /^GENERATE_LATEX/cGENERATE_LATEX         = NO >> ${SEDSCRIPT}
	echo /^USE_MDFILE_AS_MAINPAGE/cUSE_MDFILE_AS_MAINPAGE = README.md >> ${SEDSCRIPT}
	sed -i -f ${SEDSCRIPT} ${DOXYGENCFG}
	rm  -f  ${SEDSCRIPT}

#
# Clean the generated documentation
#
docs-clean:
	rm -rf html latex && echo Done.

#
#
#
cproto:
	cproto -c ${addprefix -I ,${INCLUDEPATH}} -D${PARTCLASS} ${SRCFILES}

#
# generates a call graph
#
cflow:
	(cflow ${CFLOWFLAGS} -D${PART} ${addprefix -I ,${INCLUDEPATH}} ${SRCFILES} 2>&1} | egrep -v "^cflow"


#
#
# opens a window with a terminal
#
term:
	${TERMAPP} -- ${TTYPROG}  ${TTYPARMS} 

#
# These labels are not files !!!
#
.PHONY: burn cflow clean cproto ddd debug default deploy disassembly docs docs-clean
.PHONY: doxygen dump edit flash force-flash gdb gdbserver help nemiver nm size tui usage
.PHONY: FORCE

# Force run
FORCE:

#
# Dependencies
#
-include ${OBJFILES:%.o=%.d}

