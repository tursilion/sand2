##########################################################################################
# HAM Makefile
#
# Created by Visual HAM V2 - Get Visual HAM at www.console-dev.de
#
##########################################################################################
#
# Here are the gfx2gba options, since they're very often needed and they're just
# too much to keep all in mind ...
include ./master.mak

#
# Set the name of your desired GBA image name here
#
PROGNAME=GBAGSM
GAMETITLE=SAND_DEMO

#ADD_LIBS=-lgsm
#
# Compile using Krawall software (set to yes or no) ?
# Also specify if Krawall is registered (yes or no)
#
USE_KRAWALL=no
KRAWALL_IS_REGISTERED=no
KRAWALL_FILES=

# Specify a custom lnkscript - need both of these lines
USE_CUSTOM_LNKSCRIPT=yes
CUSTOM_LNKSCRIPT=lnkscript

#
# Set a list of files you want to compile
#
# Data files are set as obj so they aren't cleaned out by the standard makefile
OFILES += main.o main_ram.o combined.o tursipic.o sample.obj desertgfx.obj tp1.obj tp2.obj thunder.obj myfont.obj presented.obj moreton.obj tangalooma.obj sine.obj sand2.obj atorestart.obj

main_ram.o : main_ram.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -I c:/work/ham/gcc-arm/include -I c:/work/ham/gcc-arm/arm-thumb-elf/include -I c:/work/ham/include -I c:/work/ham/system -c -DNDEBUG -O3 -marm -mthumb-interwork -Wall -save-temps -funroll-loops -fverbose-asm -nostartfiles main_ram.c -o main_ram.o

combined.o : combined.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -I c:/work/ham/gcc-arm/include -I c:/work/ham/gcc-arm/arm-thumb-elf/include -I c:/work/ham/include -I c:/work/ham/system -c -DNDEBUG -O3 -mthumb-interwork -Wall -save-temps -funroll-loops -fverbose-asm -nostartfiles combined.c -o combined.o
	
sine.obj : sine.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles sine.c -o sine.obj
	
sample.obj : sand16.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles sand16.c -o sample.obj

#sample.obj : dan_tune1.c
#	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles dan_tune1.c -o sample.obj
#sample.obj : kittyn16.c
#	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles kittyn16.c -o sample.obj
#sample.obj  : Pent.c
#	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles Pent.c -o sample.obj


desertgfx.obj : desert24-bit.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles desert24-bit.c -o desertgfx.obj
	
sand2.obj : sand2.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles sand2.c -o sand2.obj

myfont.obj: font2.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles font2.c -o myfont.obj

moreton.obj: moreton.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles moreton.c -o moreton.obj
tangalooma.obj: tangalooma.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles tangalooma.c -o tangalooma.obj
atorestart.obj: atorestart.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles atorestart.c -o atorestart.obj

# Files for the Tursi intro
tp1.obj : tp1.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles tp1.c -o tp1.obj
tp2.obj : tp2.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles tp2.c -o tp2.obj
thunder.obj: thunder16.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles thunder16.c -o thunder.obj
presented.obj: presented.c
	c:/work/ham/gcc-arm/bin/arm-thumb-elf-gcc.exe -c -I c:/work/ham/include -DNDEBUG -mthumb-interwork -Wall -nostartfiles presented.c -o presented.obj

##########################################################################################
# Standard Makefile targets start here
##########################################################################################
all : $(PROGNAME).$(EXT) clean

#
# Most Makefile targets are predefined for you, suchas
# vba, clean ... in the following file
#
include $(HAMDIR)/system/standard-targets.mak

##########################################################################################
# Custom  Makefile targets start here
##########################################################################################

gfx: makefile
#	gfx2gba -t8 -m -fsrc -ogfx gfx\\bitmap.bmp
#	gfx2gba -t8 -m -fsrc -rs -ogfx gfx\\bitmap.bmp



