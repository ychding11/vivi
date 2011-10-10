#include "printk.h"
#include "machine.h"

#include <string.h>

typedef unsigned long DWORD ;

#define TPRINTK(arg...) printk("\t"##arg)

void delay_50ms(int delay);

static int packet_length = 0;
#define MAX_PACKET_LENGTH (256)

#include "ide.h"

#define IOBASE 0x20000000
#define MECR (*(volatile unsigned long*)(0xA0000018))

#define IDE_DATA_REG (IOBASE + 0x40)
#define IDE_ERROR_REG (IOBASE + 0x41)
#define IDE_FEATURE_REG IDE_ERROR_REG
#define IDE_NSECTOR_REG (IOBASE + 0x42)
#define IDE_SECTOR_REG (IOBASE + 0x43)
#define IDE_LCYL_REG (IOBASE + 0x44)
#define IDE_HCYL_REG (IOBASE + 0x45)
#define IDE_SELECT_REG (IOBASE + 0x46)
#define IDE_STATUS_REG (IOBASE + 0x47)
#define IDE_COMMAND_REG IDE_STATUS_REG

#define IDE_CONTROL_REG (IOBASE + 0xc6)
#define IDE_ALTSTATUS_REG IDE_CONTROL_REG

void ide_fixstring (unsigned char *s, const int bytecount, const int byteswap)
{
	unsigned char *p = s, *end = &s[bytecount & ~1];

	if (byteswap) {
		for (p = end ; p != s;) {
			unsigned short *pp = (unsigned short *) (p -= 2);
			*pp = ___swab16(*pp);
		}
	}

	while (s != end && *s == ' ')
		++s;

	while (s != end && *s) {
		if (*s++ != ' ' || (s != end && *s && *s != ' '))
			*p++ = *(s-1);
	}

	while (p != end)
		*p++ = '\0';
}

char * get_medium_str(unsigned char type)
{
	if (type > 0x04) type -= 0x04;
	switch(type) {
		case 0x01 : return "data";
		case 0x02 : return "audio";
		case 0x03 : return "mixed";
		case 0x04 : return "hybrid";
		default: return "Unknown";
	}
}

int wait_for_drq(unsigned char good, unsigned char bad)
{
	unsigned char stat;
	unsigned int try=200;
	int i,j;

	while (!OK_STAT(stat=GET_STAT(),good,bad)) {
//		IDE_DELAY;
//		printk(__FUNCTION__"(): stat = %x, %x, %x\n", stat, good, bad);
//		printk(",");

		for (i=1000;i>0;i--) j=i;

		if (try)
			try--;
		else {
			return 1;
		}
	}

	return 0;
}


void init_pc(unsigned char *pc, unsigned char cmd, unsigned short bc)
{
	memset(pc, 0, MAX_PACKET_LENGTH);

	pc[0] = cmd;

	WRITE_U8(SELECT_ALL, IDE_SELECT_REG);

	WRITE_U8(0x00, IDE_FEATURE_REG);

	WRITE_U8(0x00, IDE_NSECTOR_REG);
	WRITE_U8(0x00, IDE_SECTOR_REG);
	WRITE_U8(bc & 0xff, IDE_LCYL_REG);
	WRITE_U8((bc>>8) & 0xff, IDE_HCYL_REG);

	WRITE_U8(0x08, IDE_CONTROL_REG);

	WRITE_U8(0xa0, IDE_COMMAND_REG); // packet command

#if 0
	while ((READ_U8(IDE_STATUS_REG) & BUSY_STAT)) {
		IDE_DELAY;
		printk("!\n");
	}
#endif
}

void wait_for_ready(unsigned char extra)
{
	unsigned char stat;
	int i=100,j;

	do {
		stat = READ_U8(IDE_STATUS_REG);
		for (i=100;i>0;i--) j=i;
		printk(__FUNCTION__"(): stat = %x\n", stat);
		IDE_DELAY;
	} while (stat & (BUSY_STAT | extra));
}

void write_packet(unsigned char *pc)
{
	int i;
	unsigned short *pc_w = (unsigned short*)pc;
	unsigned short cmd;

	printk("\t");
	for (i=0;i<(packet_length / 2); i++) {
//		cmd = ___swab16(*pc_w);
		cmd = *pc_w;
		printk("%04x ", cmd);
		WRITE_U16(cmd, IDE_DATA_REG);
		pc_w++;
	}
	printk("\n");
}

void read_packet_data(unsigned short *buf, int size)
{
	int i;

//	printk("size = %d, bc = %d\n",
//			size, READ_U8(IDE_LCYL_REG) , (READ_U8(IDE_HCYL_REG)<<8));

	for (i=0;i<size;i++) {
		*buf = (READ_U16(IDE_DATA_REG));
//		printk(__FUNCTION__"(): buf = %x\n", *buf);
		buf++;
	}
}

void block_read_command(unsigned long block, unsigned char rsector, unsigned char cmd)
{
		WRITE_U8(0x08, IDE_CONTROL_REG);
		WRITE_U8(rsector, IDE_NSECTOR_REG);
		WRITE_U8(block & 0xff, IDE_SECTOR_REG);
		WRITE_U8((block >> 8) & 0xff, IDE_LCYL_REG);
		WRITE_U8((block >> 16) & 0xff, IDE_HCYL_REG);
		WRITE_U8(((block >> 24) & 0x0f) | SELECT_0, IDE_SELECT_REG);

		WRITE_U8(cmd, IDE_COMMAND_REG); // command

		do {
//			IDE_DELAY;
		} while ((READ_U8(IDE_STATUS_REG) & BUSY_STAT));
}

void delay_50ms(int delay)
{
	printk("\t");
	for (; delay; delay--) {
		IDE_DELAY;
//		printk(".");
	}
	printk("\n");
}

void eject(void)
{
	unsigned char pc[MAX_PACKET_LENGTH];

	init_pc(&pc[0], 0x1b, 0);
	pc[4] = 0x02 + 0;
	write_packet(&pc[0]);
}


int main(void)
{
	struct hd_driveid driveid;
	DWORD hd_status;
	int media_type = 0;

	printk("\n\n\n");

/*
 * step0:
 */
	{
			printk("status = 0x%02x\n",  READ_U8(IDE_STATUS_REG));
			printk("error = 0x%02x\n", READ_U8(IDE_ERROR_REG));


		printk("[+] step #0 : wait for IDE stability\n");

//		delay_50ms(10);

		MECR = 0x994A18c6;

		printk("\tIOBASE  = 0x%08x\n", IOBASE);
		printk("\tMECR  = 0x%08x\n", MECR);

		printk("[-] step #0 : end\n\n");

#if 0 // dump REGs
		printk("DATA   : %04x\n" , READ_U16(IOBASE + IDE_DATA));
		printk("ERRFEA : %02x\n" , READ_U8(IOBASE + IDE_ERRFEA));
		printk("SCTCT  : %02x\n" , READ_U8(IOBASE + IDE_SCTCT));
		printk("SCT    : %02x\n" , READ_U8(IOBASE + IDE_SCT));
		printk("CLNLOW : %02x\n" , READ_U8(IOBASE + IDE_CLNLOW));
		printk("CLNHIGH: %02x\n" , READ_U8(IOBASE + IDE_CLNHIGH));
		printk("HEAD   : %02x\n" , READ_U8(IOBASE + IDE_HEAD));
		printk("STCMD  : %02x\n" , READ_U8(IOBASE + IDE_STCMD));
#endif
	}

/*
 * step1:
 */
	{
		printk("[+] step #1 : select all ide\n");

		WRITE_U8(SELECT_ALL, IDE_SELECT_REG);

		IDE_DELAY;

		if (READ_U8(IDE_SELECT_REG) != SELECT_ALL) {
			printk("\nselect all failed ? : %x != %x\n", 
					READ_U8(IDE_SELECT_REG), SELECT_ALL);
		}
		printk("[-] step #1 : end\n\n");
	}

/*
 * step2:
 */
	{
		unsigned char a,s;
		int stat;

		printk("[+] step #2 : send identify command\n");

		IDE_DELAY;

		a = READ_U8(IDE_ALTSTATUS_REG);
		s = READ_U8(IDE_STATUS_REG);

		if ( (a ^ s) & ~INDEX_STAT )
			hd_status = IDE_ALTSTATUS_REG;
		else
			hd_status = IDE_STATUS_REG;

		WRITE_U8(0xec, IDE_COMMAND_REG); // command : identify - ide-disk

		do {
			IDE_DELAY;
		} while (READ_U8(hd_status) & BUSY_STAT);

		if (!OK_STAT(stat=GET_STAT(),DRQ_STAT,BAD_R_STAT)) {
			WRITE_U8(0xa1, IDE_COMMAND_REG); // command : pidentify - ide-cd
			IDE_DELAY;
			do {
				IDE_DELAY;
			} while (READ_U8(hd_status) & BUSY_STAT);
			if (!OK_STAT(stat=GET_STAT(),DRQ_STAT,BAD_R_STAT)) {
				printk(" failed. stat=%x (good=%x, bad=%x)\n",
						stat, DRQ_STAT, BAD_R_STAT);
				return 0;
			}
		} else media_type = IDE_DISK;

		printk("[-] step #2 : end\n\n");
	}

/*
 * step3:
 */
	{
		unsigned short * dptr = (unsigned short*)&driveid;
		int i = 0;
		unsigned short aaa,bbb;

		printk("[+] step #3 : do identify\n\n");

		for (i=0;i<sizeof(driveid)/2 -1;i++)
		{
			*dptr = READ_U16(IDE_DATA_REG) & 0xffff;
			dptr++;
		}

		if (!media_type) 
			media_type = (driveid.config >> 8) & 0x1f;

		TPRINTK("Drive type        : ");
		switch(media_type) {
			case IDE_FLOPPY:
				printk("Floppy"); break;
			case IDE_CDROM:
				printk("CD/DVD-ROM"); break;
			case IDE_TAPE:
				printk("Tape"); break;
			case IDE_OPTICAL:
				printk("Optical"); break;
			case IDE_DISK:
				printk("ATA-Disk"); break;
			default:
				printk("Unknown (type %x)", media_type); break;
		}
		printk(" drive\n");

		ide_fixstring (&driveid.model[0],     sizeof(driveid.model),     1);
		ide_fixstring (&driveid.fw_rev[0],    sizeof(driveid.fw_rev),    1);
		ide_fixstring (&driveid.serial_no[0], sizeof(driveid.serial_no), 1);

		driveid.model[sizeof(driveid.model)-1] = '\0';
		driveid.fw_rev[sizeof(driveid.fw_rev)-1] = '\0';
		driveid.serial_no[sizeof(driveid.serial_no)-1] = '\0';
		TPRINTK("Model name        : %s\n", driveid.model);
		TPRINTK("Firmware revision : %s\n", driveid.fw_rev);
		TPRINTK("Serial number     : %s\n", driveid.serial_no);
		
		printk("\n");

		aaa = driveid.config;
		TPRINTK("Protocol Type     : ");
		bbb = (aaa >> 14) & 0x03;
		switch(bbb) {
			case 0x02 : printk("ATAPI\n"); break;
			case 0x03 : printk("Unknown (reserved?)\n"); break;
			default: printk("ATA\n"); break;
		}
		TPRINTK("DRQ type          : ");
		bbb = (aaa >> 5) & 0x03;
		switch(bbb) {
			case 0x00 : printk("Microprocessor DRQ\n"); break;
			case 0x01 : printk("Interrupt DRQ\n"); break;
			case 0x02 : printk("Accelerated DRQ\n"); break;
			case 0x03 : 
			default:
						printk("Unknown (0x%02x\n", bbb); break;
		}
		TPRINTK("Packet length     : ");
		bbb = (aaa & 0x03);
		switch(bbb) {
			case 0x00 : printk("12 bytes\n"); packet_length = 12; break;
			case 0x01 : printk("16 bytes\n"); packet_length = 16; break;
			default: printk("Unknown (0x%02x)\n", bbb); break;
		}
		TPRINTK("Removable media   : ");
		bbb = (aaa >> 7) & 0x01;
		if (bbb)
			printk("Yes\n");
		else
			printk("No\n");

		printk("\n[-] step #3 : end\n\n");

		if (media_type == IDE_DISK) 
			goto hdd;
		else {
			if (media_type == IDE_CDROM) {
				goto cdrom;
			}
			else
				return 0;
		}
	}

////////////////
// IDE-DISK
////////////////
hdd:

/*
 * step4:
 */
	{
		unsigned short lba_cyls = driveid.cyls;
		unsigned short lba_sects = driveid.sectors;
		unsigned short lba_heads = driveid.heads;

		printk("[+] step #4 : show info\n");

		if (lba_cyls > 1024) { 
			lba_heads = 255; 
			lba_cyls = driveid.lba_capacity / (lba_heads * lba_sects);
		}

		printk("\tphysical cyls  : %6d(CHS) %6d(LBA)\n", 
				driveid.cyls, lba_cyls);
		printk("\tphysical heads : %6d(CHS) %6d(LBA)\n", 
				driveid.heads, lba_heads);
		printk("\tphysical sects : %6d(CHS) %6d(LBA)\n", 
				driveid.sectors, lba_sects);
		printk("\tlba total sects: %d\n", driveid.lba_capacity);

		hda_dev.ssize = MAX_SECTOR_SIZE;
		hda_dev.heads = lba_sects;
		hda_dev.sectors = driveid.lba_capacity;

		printk("[-] step #4 : end\n\n");
	}

/*
 * step5:
 */
	{
		unsigned char MBR[MAX_SECTOR_SIZE];
		unsigned short *dptr = (unsigned short*)&MBR[0];
		unsigned long block = 0;
		unsigned long nsector = 2;
		unsigned char stat;
		int i,j;

		printk("[+] step #5 : parse partition table of MBR\n");

		block_read_command(block, nsector, 0x20);
		
/*
 *  retry:
 */
		if (!OK_STAT(stat=READ_U8(IDE_STATUS_REG),DRQ_STAT,BAD_R_STAT)) {
			printk("stat = %x, good = %x, bad = %x\n",
						stat, DRQ_STAT, BAD_R_STAT);

			if (stat & (ERR_STAT|DRQ_STAT)) {
				printk("\tfailed.\n");
				printk("\t(ERR_STAT|DRQ_STAT) = %x\n", (ERR_STAT|DRQ_STAT));
				printk("\tERRFEA : %02x\n" , READ_U8(IOBASE + IDE_ERRFEA));
				printk("\tSCTCT  : %02x\n" , READ_U8(IOBASE + IDE_SCTCT));
				printk("\tSCT    : %02x\n" , READ_U8(IOBASE + IDE_SCT));
				printk("\tCLNLOW : %02x\n" , READ_U8(IOBASE + IDE_CLNLOW));
				printk("\tCLNHIGH: %02x\n" , READ_U8(IOBASE + IDE_CLNHIGH));
				printk("\tHEAD   : %02x\n" , READ_U8(IOBASE + IDE_HEAD));
				printk("\tSTCMD  : %02x\n" , READ_U8(IOBASE + IDE_STCMD));
				return 0;
			}
			IDE_DELAY;
			return 0;
//			goto retry;
		}

		memset(&MBR[0], 0, sizeof(MBR));

		for (i=0;i<sizeof(MBR)/2;i++) {
			*dptr = READ_U16(IDE_DATA_REG) & 0xffff;
			dptr++;
		}
#if 0 // dump MBR
		for (i=0;i<sizeof(MBR);i++) {
			if ((i % 16) == 0) printk("\t");
			printk("%02x ", MBR[i]);
			if (((i+1) % 16) == 0) {
				printk("  ");
				for (j=i-15; j<=i; j++)
					if (isalnum(MBR[j]) || (MBR[j]>127)) 
						printk("%c", MBR[j]);
					else
						printk(" ");

				printk("\n");
			}
		}
#endif
		for (i = 0; i < MAXIMUM_PARTS; i++) {
			struct pte *pe = (struct pte*)&ptes[i];

			pe->part_table = pt_offset(MBR, i);
			pe->ext_pointer = 0;
			pe->changed = 0;
			pe->offset = 0;
			pe->sectorbuffer = MBR;
		}

		list_table(&hda_dev);


		printk("[-] step #5 : end\n\n");
	}

//////////////////
// IDE-CD
//////////////////
cdrom:
  {
	unsigned char nframes = 0;
	unsigned char pc[MAX_PACKET_LENGTH];
	unsigned short leadout;
	__u32 kernel_start;
	char try = 30;

/*
 * cdrom_step4: 
 */

	{
		struct atapi_capabilities_page cap;
		struct {
			__u32 lba;
			__u32 length;
		} capbuf;
		unsigned long tmp;
		unsigned int cur_speed = 0;
		unsigned int max_speed = 0;
		unsigned short aaa;
		unsigned short bbb;
		int word54_valid = 0;
		int word64_valid = 0;

		printk("[+] step #3.5 : show cdrom specific device info\n\n");

		aaa =  ___swab16(driveid.capability) ;
		TPRINTK("Capability        : ");
		if (aaa & 0xff) 
			printk("Unknown (vendor specific)\n");
		else
			printk("%sDMA %sLBA %sIORDY IORDY_can%s_be_disabled %sOverlap\n",
				aaa & (1<<8) ? "" : "no-", aaa & (1<<9) ? "" : "no-",
				aaa & (1<<11) ? "" : "no-", aaa & (1<<10) ? "" : "not",
				aaa & (1<<13) ? "" : "no-");
		aaa = driveid.tPIO;
		TPRINTK("PIO speed         : %s (0x%04x) \n",
			aaa==0 ? "slow":(aaa==1 ? "medium":(aaa==2 ? "fast":"unknown")),
			aaa);
		aaa = driveid.tDMA;
		TPRINTK("DMA speed         : %s (0x%04x)\n\n",
			aaa==0 ? "slow":(aaa==1 ? "medium":(aaa==2 ? "fast":"unknown")),
			aaa);
		aaa = ___swab16(driveid.field_valid);
		word54_valid = aaa & 0x01 ? 1 : 0;
		word64_valid = aaa & 0x02 ? 1 : 0;
		if (word64_valid) {
			aaa = ___swab16(driveid.eide_pio_modes);
			TPRINTK("EIDE PIO mode         : %sMODE3 %sMODE4\n",
				aaa & 0x1 ? "" : "no-",
				aaa & 0x2 ? "" : "no-");
			TPRINTK("EIDE PIO (no IORDY)   : %d ns\n", 
					___swab16(driveid.eide_pio));
			TPRINTK("EIDE PIO (with IORDY) : %d ns\n", 
				___swab16(driveid.eide_pio_iordy));
		}
		printk("\n");

		printk("[-] step #3.5 : end\n\n");


		printk("[+] step #4 : show info\n\n");

	// read CD-ROM capacity, 8 byte data

		wait_for_ready(0);

		init_pc(&pc[0], 0x25, sizeof(capbuf)/2); 
		write_packet(&pc[0]);

cdrom_step4_0: 

		if (wait_for_drq(DRQ_STAT, BAD_R_STAT)) {
			try--;
//			if (try) goto cdrom_step4_0;
//			else return 0;
		}

		read_packet_data((unsigned short*)&capbuf, sizeof(capbuf) / 2);

		capbuf.length = ___swab32(capbuf.length);
		capbuf.lba = ___swab32(capbuf.lba) + 1;
		printk("\tCapacity : length = 0x%04x(%d) bytes, LBA = 0x%x(%d)\n",
				capbuf.length, capbuf.length,capbuf.lba, capbuf.lba);
		leadout = capbuf.lba;
    /////////

	// mode sense
		wait_for_ready(0);

		init_pc(&pc[0], 0x5a, sizeof(cap)/2);
		pc[2] = 0x00 | 0x2A; // current value, capacity
		pc[8] = sizeof(cap) & 0xff;
		pc[7] = (sizeof(cap) >> 8) & 0xff;
		write_packet(&pc[0]);

cdrom_step4_1:

		if (wait_for_drq(DRQ_STAT, BAD_R_STAT)) {
//			printk("wait_for_drq() failed.\n");
//			return 0;
		}

		read_packet_data((unsigned short*)&cap, sizeof(cap) / 2);

		TPRINTK("Drive:\n");
		TPRINTK("\tRead  : [");
		if (cap.cd_r_read) printk(" CD-R");
		if (cap.cd_rw_read) printk(" CD-RW");
		if (cap.dvd_rom) printk(" DVD-ROM");
		if (cap.dvd_r_read) printk(" DVD-R");
		if (cap.dvd_ram_read) printk(" DVD-RAM");
		printk(" ]\n");

		TPRINTK("\tWrite : [");
		if(cap.cd_r_write) printk(" CD-R");
		if(cap.cd_rw_write) printk(" CD-RW");
		if(cap.dvd_r_write) printk(" DVD-R");
		if(cap.dvd_ram_write) printk(" DVD-RAM");
		printk(" ]\n");

		if (!driveid.model[0] && !strncmp(driveid.fw_rev, "241N", 4)) {
			cur_speed = (((unsigned int)cap.curspeed) + (176/2)) / 176;
			max_speed = (((unsigned int)cap.maxspeed) + (176/2)) / 176;
		} else {
			cur_speed = (___swab16(cap.curspeed) + (176/2)) / 176;
			max_speed = (___swab16(cap.maxspeed) + (176/2)) / 176;
		}

		TPRINTK("\tSpeed : %dX (max: %dX)\n", cur_speed, max_speed);
		TPRINTK("\tCache : %dkB\n\n", ___swab16(cap.buffer_size));

		TPRINTK("Disc :\n");
		TPRINTK("\tSize : %smm\n", 
				(cap.header.medium_type & 0x04) > 0x04 ? "80":"120");
		TPRINTK("\tType : %s\n", 
				(cap.header.medium_type >> 4) == 0x02 ? "CD-RW":"CD-R");
		TPRINTK("\tMode : %s\n", 
				(get_medium_str(cap.header.medium_type & 0x04)));

//		printk("length = %d\n", ___swab16(cap.header.length));

		printk("[-] step #4 : end\n\n");
	}

/*
 * cdrom_step5:
 */
	{
		struct atapi_toc_header toc_header;
		struct atapi_toc_entry toc_entry;
		int i;

		printk("[+] step #5 : parse track info of TOC\n");

		wait_for_ready(0);
		init_pc(&pc[0], 0x43, sizeof(toc_header)/2);
		pc[1] = 0x00;
		pc[8] = sizeof(toc_header) & 0xff;
		pc[7] = (sizeof(toc_header) >> 8)& 0xff;
		pc[9] = 0x00; // DATA format
		write_packet(&pc[0]);
cdrom_step5_1:
		if (wait_for_drq(DRQ_STAT, BAD_R_STAT)) {
//			goto cdrom_step5_1;
		}

		read_packet_data((unsigned short*)&toc_header, sizeof(toc_header)/2);

		toc_header.toc_length = ___swab16(toc_header.toc_length);

		TPRINTK("      TRACK      ADR   CTRL      LBA\n");
		for (i=0; i < (toc_header.toc_length / sizeof(toc_entry)); i++) {
			read_packet_data((unsigned short*)&toc_entry, sizeof(toc_entry)/2);
			toc_entry.msf.lba1 = ___swab16(toc_entry.msf.lba1);
			TPRINTK("%s  %4d  :    %02x     %02x   0x%04x\n", 
					toc_entry.msf.lba1 == leadout ? " lout" : "track", 
					toc_entry.track,
					toc_entry.adr, toc_entry.control, toc_entry.msf.lba1);
			if (toc_entry.track == 2) kernel_start = toc_entry.msf.lba1;
		}

		printk("[-] step #5 : end\n\n");
	
	}
  }

  return 0;
}
