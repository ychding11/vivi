/*
 * vivi/lib/load-file.c: Load a file via serial or usb.
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author : Janghoon Lyu <nandy@mizi.com>
 * Date   : $Date: 2002/10/22 08:06:11 $
 *
 * $Revision: 1.10 $
 *
 * History
 *
 * 2002-06-27: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2002-07-29: Janghoon Lyu <nandy@mizi.com>
 *    - download_file()에서 modem 선택할 때의 버그 수정.
 *
 * 2002-10-22: Janghoon Lyu <nandy@mizi.com>
 *    - Simpify functions related to a load command.
 *
 * TODO:
 *   1. default modem이라는 VIVI 파라미터가 필요할까요?
 */

#include "config.h"
#include "command.h"
#include "machine.h"
#include <priv_data.h>
#include "mtd/mtd.h"
#include "printk.h"
#include "serial.h"
#include "vivi_string.h"
#include "md5.h"
#include "xmodem.h"
#include "ymodem.h"

#include <string.h>

#define LOAD_TO_FLASH	1
#define LOAD_TO_RAM	2

static int 
modem_is(const char *mt)
{
	if (strncmp("x", mt, 1) == 0) {
		return X_MODEM;
	} else if (strncmp("y", mt, 1) == 0) {
		return Y_MODEM;
	} else if (strncmp("z", mt, 1) == 0) {
		return Z_MODEM;
	} else {
		return UNKNOWN_MODEM;
	}
}

static size_t
download_file(char *buf, size_t size, int modem, char *name)
{
	switch (modem) {
		case X_MODEM:
#ifdef CONFIG_SERIAL_XMODEM
			printk("Ready for downloading using xmodem...\n");
			printk("Waiting...\n");
			return xmodem_receive(buf, size);
#else
			printk("Not support XMODEM protocol by this vivi\n");
#endif
			break;
		case Y_MODEM:
#ifdef CONFIG_SERIAL_YMODEM
			printk("Ready for downloading using ymodem...\n");
			printk("Waiting...\n");
			return ymodem_receive(buf, size, name);
#else
			printk("Not support YMODEM protocol by this vivi\n");
#endif
			break;
		case Z_MODEM:
			printk("Not support zmodem yet.\n");
			break;
		default:
			printk("Not support this modem\n");
			break;
	}
	return 0;
}

void 
command_load(int argc, const char **argv)
{
	char *buf = (char *)RAM_BASE;
	//loff_t to; 
	unsigned long to;	/* is declared to long-long type ? */
	size_t size;
	int modem = 0;
	int where_to = 0;
	int ret;
	size_t retlen;
	mtd_partition_t *dst_part;
	int flag;
	char file_name[255] = { 0, };

	/* Parse arguments */
	switch (argc) {
	case 2:
		if (strncmp("help", argv[1], 4) == 0) {
			goto print_usage;
		} else {
			goto invalid_cmd;
		}
		break;
	case 4:
		dst_part = get_mtd_partition(argv[2]);
		if (dst_part == NULL) {
			printk("Could not found \"%s\" partition\n", argv[2]);
			return;
		}
		to = dst_part->offset;
		size = dst_part->size;
		flag = dst_part->flag;
		modem = modem_is(argv[3]);
		break;
	case 5:
		to = strtoul(argv[2], NULL, 0, &ret);
		if (ret) goto error_parse_arg;
		size = (size_t)strtoul(argv[3], NULL, 0, &ret);
		if (ret) goto error_parse_arg;
		modem = modem_is(argv[4]);
		flag = 0;
		break;
	default:
		goto invalid_cmd;
	}

	if (strncmp("flash", argv[1], 5) == 0) {
		where_to = LOAD_TO_FLASH;
	} else if (strncmp("ram", argv[1], 3) == 0) {
		where_to = LOAD_TO_RAM;
	} else {
		printk("invalid argument. 'flash' or 'ram' is available\n");
		return;
	}

	/* if load to ram */
	if (where_to == LOAD_TO_RAM) {
		buf = (char *)to;
	}

	/* download a file */
	retlen = download_file(buf, size, modem, file_name);

	/* hacked by nandy. delay for serial output */
	{ int i = 0x10000; while (i > 0) i--; }

	if (retlen == 0) {
		printk("Failed downloading file\n");
		return;
	}

	printk("Downloaded file at 0x%08lx, size = %d bytes\n", buf, retlen);

	switch (where_to) {
	case LOAD_TO_FLASH:
		if (retlen > size) {
			printk("An image size is too large to write flash.
				wanted = 0x%08lx, loaded = 0x%08lx\n", size, retlen);
			return;
		}

		if (flag & MF_ISRAM) {
			printk("Ooops, actually this partition is ram. It's tweak...!\n");
			return -1;
		}

		ret = write_to_flash((loff_t)to, retlen, buf, flag);
		break;
	case LOAD_TO_RAM:
		break;
	}

	if (modem == Y_MODEM)
		check_md5sum(buf, retlen, file_name);

	return;

error_parse_arg:
	printk("Can't parsing argumets\n");
	return;
invalid_cmd:
	printk("invalid 'load' command: too few(many) arguments\n");
print_usage:
	printk("\nUsage:\n");
	printk("  load <flash|ram> [ <partname> | <addr> <size> ] <x|y|z>\n");
	return;
}

user_command_t load_cmd = {
	"load",
	command_load,
	NULL,
	"load {...}\t\t\t-- Load a file to RAM/Flash"
};
