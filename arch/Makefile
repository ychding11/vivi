# Select CPU dependent flags.  Note that order of declaration is important;
# the options further down the list override previous items.
#
# Note!  For APCS-26 YOU MUST HAVE AN APCS-26 LIBGCC.A
#
apcs-y				:=-mapcs-32

# This selects which instruction set is used.
arch-y				:=
arch-$(CONFIG_CPU_32v4)		:=-march=armv4

# This selects how we optimise for the processor.
tune-y				:=
tune-$(CONFIG_CPU_ARM920T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_SA1100)	:=-mtune=strongarm1100

CFLAGS		+=$(apcs-y) $(arch-y) $(tune-y) -mshort-load-bytes -msoft-float
AFLAGS		+=$(apcs-y) $(arch-y) -mno-fpu -msoft-float

ifeq ($(CONFIG_CPU_32),y)
PROCESSOR	 = armv
TEXTADDR	 = 0x00000000
endif
LDSCRIPT	= arch/vivi.lds.in

ifeq ($(CONFIG_ARCH_SA1100),y)
MACHINE		 = sa1100
TEXTADDR	 = 0x00000000
endif

ifeq ($(CONFIG_ARCH_S3C2400),y)
MACHINE		= s3c2400
ifeq ($(CONFIG_S3C2400_GAMEPARK),y)
    TEXTADDR	= 0x00000000
    ifeq ($(CONFIG_S3C2400_GAMEPARK_OSSWITCH),y)
      TEXTADDR	= 0x00100000
    endif
    ifeq ($(CONFIG_S3C2400_GAMEPARK_ON_RAM),y)
      TEXTADDR	= 0x0C000000
    endif
else
   TEXTADDR	= 0x00000000
endif
endif

ifeq ($(CONFIG_ARCH_S3C2410),y)
MACHINE		= s3c2410
  ifeq ($(CONFIG_S3C2410_NAND_BOOT),y)
    TEXTADDR	= 0x33f00000
  else
    TEXTADDR	= 0x00000000
  endif
endif

ifeq ($(CONFIG_ARCH_PXA250),y)
MACHINE		 = pxa250
TEXTADDR	 = 0x00000000
endif

ifeq ($(CONFIG_VIVI_ADDR),y)
TEXTADDR	= 0x$(CONFIG_VIVI_TEXTADDR)
endif

export	MACHINE PROCESSOR TEXTADDR

HEAD		:= arch/$(MACHINE)/head.o

# If we have a machine-specific directory, then include it in the build.
MACHDIR		:= arch/$(MACHINE)
ifeq ($(MACHDIR),$(wildcard $(MACHDIR)))
SUBDIRS		+= $(MACHDIR)
CORE_FILES	:= $(MACHDIR)/$(MACHINE).o $(CORE_FILES)
endif

CLEAN_FILES += arch/vivi.lds

vivi: $(HEAD)  arch/vivi.lds

arch/vivi.lds: $(LDSCRIPT) dummy
	@sed s/TEXTADDR/$(TEXTADDR)/ $(LDSCRIPT) >$@

#
# Configuration targets.  Use these to select a
# configuration for your architecture
%_config:
	@( \
	CFG=$(@:_config=); \
	if [ -f arch/def-configs/$$CFG ]; then \
	  [ -f .config ] && mv -f .config .config.old; \
	  cp arch/def-configs/$$CFG .config; \
	  echo "*** Default configuration for $$CFG installed"; \
	  echo "*** Next, you may run 'make oldconfig'"; \
	else \
	  echo "$$CFG does not exist"; \
	fi; \
	)
