CROSS=/opt/host/armv4l/bin/armv4l-unknown-linux-

HOST_CC=gcc

CC=$(CROSS)gcc
LD=$(CROSS)ld
NM=$(CROSS)nm
OBJCOPY=$(CROSS)objcopy

VIVI_CONFIG = ../.config

TARGET = skeleton

ifneq ($(wildcard $(VIVI_CONFIG)),)
	include $(VIVI_CONFIG)
endif

ifeq ($(CONFIG_ARCH_S3C2410),y)
TEXTADDR=0x30008000
TARGET += smc_test mport3_lcd
endif
ifeq ($(CONFIG_ARCH_PXA250),y)
TEXTADDR=0xa0008000
TARGET += hello
endif
ifeq ($(CONFIG_ARCH_SA1100),y)
TEXTADDR=0xc0008000
TARGET += timer forte_ide 
endif

ifeq ($(TEXTADDR),)
error:
	@ echo -en "\n configure and build the VIVI first! \n\n"
endif

# standard header of VIVI.
INC=-I../include
DEP=Makefile $(wildcard ../include/*.h) $(VIVI_CONFIG)

# standard object of VIVI.
LIBS=../lib/heap.o ../lib/printk.o ../lib/ctype.o ../drivers/serial/serial_core.o

# C lib.
CLIBS=-L/opt/host/armv4l/lib/gcc-lib/armv4l-unknown-linux/2.95.2
CLIBS+=-lgcc -lc

CFLAGS=-Wall -mapcs-32 -msoft-float $(INC)
AFLAGS=-mno-fpu $(CFLAGS)
OBJCOPYFLAGS = -R .comment -R .stab -R .stabstr

all: $(TARGET)

head.o: %.o: %.S
	$(CC) -D__ASSEMBLY__ -D__KERNEL__ $(AFLAGS) -c $< -o $*.o

test.lds: test.lds.in $(DEP)
	sed "s|TEXTADDR|$(TEXTADDR)|" test.lds.in > test.lds

$(TARGET): %: test.lds head.o %.o $(LIBS) $(DEP)
	$(LD) -v -Ttest.lds -Bstatic head.o $(LIBS) $*.o -o $*-elf $(CLIBS)
	$(NM) -v -l $*-elf > $*.map
	$(OBJCOPY) -O binary -S $*-elf $* $(OBJCOPYFLAGS)

*.o: $(DEP)

clean:
	@rm -vf *~ *.o test.lds
	@rm -vf *elf *.map
	@rm -vf $(TARGET)


#
#
# ^^; example of calling the VIVI embedded functions
#
#

call_func: %: ../vivi.map test.lds head.o function.map %.o Makefile
	$(LD) -v -Ttest.lds -Bstatic head.o $(CLIBS) $*.o -o $*-elf
	$(NM) -v -l $*-elf > $*.map
	$(OBJCOPY) -O binary -S $*-elf $* $(OBJCOPYFLAGS)

function.map: function.list function.awk
	gawk -f function.awk < function.list > function.map
