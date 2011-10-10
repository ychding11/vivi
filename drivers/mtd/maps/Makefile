#
# linux/drivers/maps/Makefile
#
# $Id: Makefile,v 1.2 2002/07/22 09:51:36 nandy Exp $

O_TARGET	:= mapslink.o

# Chip mappings
obj-$(CONFIG_MTD_SA1100)	+= sa1100_flash.o
obj-$(CONFIG_MTD_S3C2400)	+= s3c2400_flash.o
obj-$(CONFIG_MTD_S3C2410)	+= s3c2410_flash.o
obj-$(CONFIG_MTD_PXA250)	+= pxa250_flash.o

include $(TOPDIR)/Rules.make
