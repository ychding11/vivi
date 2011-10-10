#
# linux/drivers/nand/Makefile
#
# $Id: Makefile,v 1.3 2002/08/26 11:08:40 nandy Exp $

O_TARGET	:= nandlink.o


obj-y			:=
obj-$(CONFIG_MTD_NAND_ECC)	+= nand_ecc.o
obj-$(CONFIG_MTD_SMC)		+= smc_core.o bon.o
obj-$(CONFIG_MTD_BONFS)		+= bon.o

include $(TOPDIR)/Rules.make
