O_TARGET		:= nor.o

obj-y := 
obj-$(CONFIG_MTD)		+= chipreg.o
obj-$(CONFIG_MTD_CFI)		+= cfi_probe.o
obj-$(CONFIG_MTD_AMDSTD)	+= amd_flash.o
obj-$(CONFIG_MTD_CFI_INTELEXT)	+= cfi_cmdset_0001.o
obj-$(CONFIG_MTD_GEN_PROBE)	+= gen_probe.o

include $(TOPDIR)/Rules.make
