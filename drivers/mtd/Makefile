O_TARGET		:= mtd.o

obj-$(CONFIG_MTD)	+= mtdcore.o maps/mapslink.o
obj-$(CONFIG_MTD_NOR)	+= nor/nor.o
obj-$(CONFIG_MTD_NAND)	+= nand/nandlink.o

subdir-y	:= 
subdir-$(CONFIG_MTD)		+= maps
subdir-$(CONFIG_MTD_NOR)	+= nor
subdir-$(CONFIG_MTD_NAND)	+= nand

include $(TOPDIR)/Rules.make
