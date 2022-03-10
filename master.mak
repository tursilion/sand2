### HAM Master Makefile           ###
### HAM by  Emanuel Schleussinger ###
### http://www.ngine.de           ###

#
# Initial HAM flag, do not change
#
#HAM_CFLAGS += -DHAM_HAM 

# This selects the type of libraries you want to link. Use 
# - thumb (thumb only library)
# - normal (arm thumb interworking capable library)
HAM_LIBGCC_STYLE = normal

# Set this to the directory in which you installed
# the HAM system
#
HAMDIR = c:/work/ham

# Set this dir to the one that contains your
# compiler and linker and stuff (gcc.exe)
#
GCCARM = $(HAMDIR)/gcc-arm

# Standard .o files needed for all HAM programs
# do not change
#
OFILES     =  crt0.o

# This reflects the GCC version used. Important for the library paths
# do not forget to set to new value once you upgrade.
#
HAM_PLATFORM = win32

# This reflects the toolchain versions used. Important for the library paths
# do not forget to set to new value once you upgrade.
#
HAM_VERSION_MAJOR    =2
HAM_VERSION_MINOR    =71
HAM_VERSION_BINUTILS =2.13.2.1
HAM_VERSION_GCC      =3.2.2
HAM_VERSION_INSIGHT  =5.3
HAM_VERSION_NEWLIB   =1.11.0

# make Multiboot the standard, comment out
# normal Cart booting
#
#HAM_CFLAGS += -DHAM_MULTIBOOT 

# Enable MBV2Lib support
#
#HAM_CFLAGS += -DHAM_ENABLE_MBV2LIB 

# Enable debugger support
#
# HAM_CFLAGS += -DHAM_DEBUGGER 

# My flags
#
HAM_CFLAGS += -DNDEBUG 
# Hang Problems with unroll-loops
#-funroll-loops 

# Enable GDB symbol support, optional. This will also reduce 
# optimization levels to allow correct in-source debugging
ifeq ($(HAMLIBCOMPILE),1)
else
ifeq ($(MAKECMDGOALS),gdb)
HAM_CFLAGS += -O0 -g  
else
HAM_CFLAGS += -O3
endif
endif

# Enable libHAM support
#
#HAM_CFLAGS += -DHAM_WITH_LIBHAM 

#
# Emulator of choice
#
HAM_EMUPATH = C:/emu/gba/visualboyadvance.exe 

#
# Libs to link into your binaries (remove -lham to link without HAMlib)
# (note that the double appearance of lgcc is on purpose to deal with sprintf)
#
#LD_LIBS = -lham -lm -lstdc++ -lsupc++ -lgcc -lc -lgcc
LD_LIBS = -lham -lm -lgcc -lc -lgcc

# --------------------------------------------------------------------------------
# --------------------------------------------------------------------------------

# These options are for setting up the compiler
# You should usually not need to change them.
# Unless you want to tinker with the compiler
# options. 
#


PREFIX      = bin
EXEC_PREFIX = arm-thumb-elf-

ifeq ($(HAM_PLATFORM),win32)
EXEC_POSTFIX = .exe  
else
EXEC_POSTFIX = 
endif

INCDIR      = -I $(GCCARM)/include -I $(GCCARM)/arm-thumb-elf/include -I $(HAMDIR)/include -I $(HAMDIR)/system 
LIBDIR      = -L $(GCCARM)/lib/gcc-lib/arm-thumb-elf/$(HAM_VERSION_GCC)/$(HAM_LIBGCC_STYLE) -L $(GCCARM)/arm-thumb-elf/lib/$(HAM_LIBGCC_STYLE) -L $(GCCARM)/lib 
#CFLAGS      = $(INCDIR) -c $(HAM_CFLAGS) -mthumb-interwork -mlong-calls -Wall -save-temps -fverbose-asm -nostartfiles
CFLAGS      = $(INCDIR) -c $(HAM_CFLAGS) -mthumb-interwork -Wall -save-temps -fverbose-asm -nostartfiles
LDFLAGS     = 
ASFLAGS     = -mthumb-interwork 
PATH       +=;$(GCCARM) 
CC          = $(GCCARM)/$(PREFIX)/$(EXEC_PREFIX)gcc$(EXEC_POSTFIX) 
GDB         = $(HAMDIR)/gcc-arm/bin/$(EXEC_PREFIX)insight$(EXEC_POSTFIX) 
AS          = $(GCCARM)/$(PREFIX)/$(EXEC_PREFIX)as$(EXEC_POSTFIX)
LD          = $(GCCARM)/$(PREFIX)/$(EXEC_PREFIX)ld$(EXEC_POSTFIX) 
OBJCOPY     = $(GCCARM)/$(PREFIX)/$(EXEC_PREFIX)objcopy$(EXEC_POSTFIX) 
EXT         = gba 
ifeq ($(HAM_PLATFORM),win32)
SHELL       = $(HAMDIR)/tools/$(HAM_PLATFORM)/sh$(EXEC_POSTFIX) 
endif
PATH       := $(GCCARM)/$(PREFIX):$(PATH)


