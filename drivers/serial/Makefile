O_TARGET		:= serial.o

obj-y := serial_core.o
obj-$(CONFIG_SERIAL_GETCMD_STD)	+= getcmd.o
obj-$(CONFIG_SERIAL_GETCMD_EXT)	+= getcmd_ext.o
obj-$(CONFIG_SERIAL_TERM)	+= term.o
obj-$(CONFIG_SERIAL_XMODEM)	+= xmodem.o
obj-$(CONFIG_SERIAL_YMODEM)	+= ymodem.o
obj-$(CONFIG_SERIAL_ZMODEM)	+= zmodem.o

include $(TOPDIR)/Rules.make
