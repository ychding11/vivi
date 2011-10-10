O_TARGET := lib.o

obj-y := boot_kernel.o heap.o memory.o
obj-y += command.o exec.o
obj-y += ctype.o printk.o
obj-y += misc.o
obj-y += load_file.o
obj-y += string.o
obj-y += time.o
obj-$(CONFIG_RESET_HANDLING) += reset_handle.o
obj-$(CONFIG_MEMORY_RAM_TEST) += memtst.o
obj-$(CONFIG_MD5) += md5.o

subdir-y	:= priv_data

include $(TOPDIR)/Rules.make
