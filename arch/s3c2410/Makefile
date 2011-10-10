O_TARGET	:= s3c2410.o

obj-y :=
# 꼭 제일 앞에 위치하도록 해주세요.
ifeq ($(CONFIG_S3C2410_NAND_BOOT),y)
obj-y += nand_read.o
endif
obj-y += mmu.o proc.o
obj-$(CONFIG_S3C2410_SMDK) += smdk.o
ifeq ($(CONFIG_S3C2410_SMDK),y)
obj-$(CONFIG_TEST) += smdk2410_test.o
endif
obj-$(CONFIG_S3C2410_MPORT3) += mport3.o
obj-$(CONFIG_S3C2410_MPORT1) += mport1.o
obj-$(CONFIG_S3C2410_MP3) +=2410-mp3.o

include $(TOPDIR)/Rules.make
