#define READ_U16(a) (*(volatile unsigned short *)(a))
#define READ_U8(a) (*(volatile unsigned char *)(a))
#define WRITE_U16(value, a) ((*(volatile unsigned short *)(a)) = value)
#define WRITE_U8(value, a) ((*(volatile unsigned char *)(a)) = value)

#define IDE_SCSI    0x21
#define IDE_DISK    0x20
#define IDE_OPTICAL 0x7
#define IDE_CDROM   0x5
#define IDE_TAPE    0x1
#define IDE_FLOPPY  0x0


enum {
	IDE_DATA = 0x40,
	IDE_ERRFEA,
	IDE_SCTCT,
	IDE_SCT,
	IDE_CLNLOW,
	IDE_CLNHIGH,
	IDE_HEAD,
	IDE_STCMD,
	IDE_ALT = 0xc6,
	IDE_ADDR
};

#define SELECT_ALL	0xa0
#define SELECT_0	0xe0

#define SELECT_DRIVE(value, addr) (WRITE_U8(value, addr))
#define GET_STAT()              READ_U8(IDE_STATUS_REG)
#define OK_STAT(stat,good,bad)  (((stat)&((good)|(bad)))==(good))


#define IDE_DELAY {long i, j=0;for (i=100000;i>0;i--) j=j;}
//#define IDE_DELAY (void)(0)



// linux/hdreg.h
#define ERR_STAT		0x01
#define INDEX_STAT		0x02
#define ECC_STAT		0x04    /* Corrected error */
#define DRQ_STAT		0x08
#define SEEK_STAT 		0x10
#define WRERR_STAT		0x20
#define READY_STAT 		0x40
#define BUSY_STAT		0x80
#define BAD_R_STAT		(BUSY_STAT   | ERR_STAT)

struct hd_driveid {
	unsigned short	config;		/* lots of obsolete bit flags */
	unsigned short	cyls;		/* "physical" cyls */
	unsigned short	reserved2;	/* reserved (word 2) */
	unsigned short	heads;		/* "physical" heads */
	unsigned short	track_bytes;	/* unformatted bytes per track */
	unsigned short	sector_bytes;	/* unformatted bytes per sector */
	unsigned short	sectors;	/* "physical" sectors per track */
	unsigned short	vendor0;	/* vendor unique */
	unsigned short	vendor1;	/* vendor unique */
	unsigned short	vendor2;	/* vendor unique */
	unsigned char	serial_no[20];	/* 0 = not_specified */
	unsigned short	buf_type;
	unsigned short	buf_size;	/* 512 byte increments; 0 = not_specified */
	unsigned short	ecc_bytes;	/* for r/w long cmds; 0 = not_specified */
	unsigned char	fw_rev[8];	/* 0 = not_specified */
	unsigned char	model[40];	/* 0 = not_specified */
	unsigned char	max_multsect;	/* 0=not_implemented */
	unsigned char	vendor3;	/* vendor unique */
	unsigned short	dword_io;	/* 0=not_implemented; 1=implemented */
	unsigned char	vendor4;	/* vendor unique */
	unsigned char	capability;	/* bits 0:DMA 1:LBA 2:IORDYsw 3:IORDYsup*/
	unsigned short	reserved50;	/* reserved (word 50) */
	unsigned char	vendor5;	/* vendor unique */
	unsigned char	tPIO;		/* 0=slow, 1=medium, 2=fast */
	unsigned char	vendor6;	/* vendor unique */
	unsigned char	tDMA;		/* 0=slow, 1=medium, 2=fast */
	unsigned short	field_valid;	/* bits 0:cur_ok 1:eide_ok */
	unsigned short	cur_cyls;	/* logical cylinders */
	unsigned short	cur_heads;	/* logical heads */
	unsigned short	cur_sectors;	/* logical sectors per track */
	unsigned short	cur_capacity0;	/* logical total sectors on drive */
	unsigned short	cur_capacity1;	/*  (2 words, misaligned int)     */
	unsigned char	multsect;	/* current multiple sector count */
	unsigned char	multsect_valid;	/* when (bit0==1) multsect is ok */
	unsigned int	lba_capacity;	/* total number of sectors */
	unsigned short	dma_1word;	/* single-word dma info */
	unsigned short	dma_mword;	/* multiple-word dma info */
	unsigned short  eide_pio_modes; /* bits 0:mode3 1:mode4 */
	unsigned short  eide_dma_min;	/* min mword dma cycle time (ns) */
	unsigned short  eide_dma_time;	/* recommended mword dma cycle time (ns) */
	unsigned short  eide_pio;       /* min cycle time (ns), no IORDY  */
	unsigned short  eide_pio_iordy; /* min cycle time (ns), with IORDY */
	unsigned short	words69_70[2];	/* reserved words 69-70 */
	/* HDIO_GET_IDENTITY currently returns only words 0 through 70 */
	unsigned short	words71_74[4];	/* reserved words 71-74 */
	unsigned short  queue_depth;	/*  */
	unsigned short  words76_79[4];	/* reserved words 76-79 */
	unsigned short  major_rev_num;	/*  */
	unsigned short  minor_rev_num;	/*  */
	unsigned short  command_set_1;	/* bits 0:Smart 1:Security 2:Removable 3:PM */
	unsigned short  command_set_2;	/* bits 14:Smart Enabled 13:0 zero */
	unsigned short  cfsse;		/* command set-feature supported extensions */
	unsigned short  cfs_enable_1;	/* command set-feature enabled */
	unsigned short  cfs_enable_2;	/* command set-feature enabled */
	unsigned short  csf_default;	/* command set-feature default */
	unsigned short  dma_ultra;	/*  */
	unsigned short	word89;		/* reserved (word 89) */
	unsigned short	word90;		/* reserved (word 90) */
	unsigned short	CurAPMvalues;	/* current APM values */
	unsigned short	word92;		/* reserved (word 92) */
	unsigned short	hw_config;	/* hardware config */
	unsigned short  words94_125[32];/* reserved words 94-125 */
	unsigned short	last_lun;	/* reserved (word 126) */
	unsigned short	word127;	/* reserved (word 127) */
	unsigned short	dlf;		/* device lock function
					 * 15:9	reserved
					 * 8	security level 1:max 0:high
					 * 7:6	reserved
					 * 5	enhanced erase
					 * 4	expire
					 * 3	frozen
					 * 2	locked
					 * 1	en/disabled
					 * 0	capability
					 */
	unsigned short  csfo;		/* current set features options
					 * 15:4	reserved
					 * 3	auto reassign
					 * 2	reverting
					 * 1	read-look-ahead
					 * 0	write cache
					 */
	unsigned short	words130_155[26];/* reserved vendor words 130-155 */
	unsigned short	word156;
	unsigned short	words157_159[3];/* reserved vendor words 157-159 */
	unsigned short	words160_255[95];/* reserved words 160-255 */
};

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned long __u32;
typedef unsigned int uint;
typedef unsigned char byte;

#define ___swab16(x) \
	({ \
		__u16 __x = (x); \
		((__u16)( \
		(((__u16)(__x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(__x) & (__u16)0xff00U) >> 8) )); \
	})

#define ___swab32(x) \
	({ \
		__u32 __x = (x); \
		((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
	})

#define MAX_SECTOR_SIZE 1024
#define MAXIMUM_PARTS   2

struct partition {
	unsigned char boot_ind;         /* 0x80 - active */
	unsigned char head;             /* starting head */
	unsigned char sector;           /* starting sector */
	unsigned char cyl;              /* starting cylinder */
	unsigned char sys_ind;          /* What partition type */
	unsigned char end_head;         /* end head */
	unsigned char end_sector;       /* end sector */
	unsigned char end_cyl;          /* end cylinder */
	unsigned char start4[4];        /* starting sector counting from 0 */
	unsigned char size4[4];         /* nr of sectors in partition */
};

struct pte {
	struct partition *part_table;	/* points into sectorbuffer */
	struct partition *ext_pointer;	/* points into sectorbuffer */
	char changed;		/* boolean */
	uint offset;		/* disk sector number */
	char *sectorbuffer;	/* disk sector contents */
} ptes[MAXIMUM_PARTS];

struct systypes {
	unsigned char type;
	char *name;
};

struct systypes i386_sys_types[] = {
	{0x00, "Empty"},
	{0x01, "FAT12"},
	{0x04, "FAT16 <32M"},
	{0x05, "Extended"},		/* DOS 3.3+ extended partition */
	{0x06, "FAT16"},		/* DOS 16-bit >=32M */
	{0x07, "HPFS/NTFS"},	/* OS/2 IFS, eg, HPFS or NTFS or QNX */
	{0x0b, "Win95 FAT32"},
	{0x0c, "Win95 FAT32 (LBA)"},/* LBA really is `Extended Int 13h' */
	{0x0e, "Win95 FAT16 (LBA)"},
	{0x0f, "Win95 Ext'd (LBA)"},
	{0x11, "Hidden FAT12"},
	{0x14, "Hidden FAT16 <32M"},
	{0x16, "Hidden FAT16"},
	{0x17, "Hidden HPFS/NTFS"},
	{0x18, "AST Windows swapfile"},
	{0x1b, "Hidden Win95 FAT32"},
	{0x1c, "Hidden Win95 FAT32 (LBA)"},
	{0x1e, "Hidden Win95 FAT16 (LBA)"},
	{0x3c, "PartitionMagic recovery"},
	{0x63, "GNU HURD or SysV"},	/* GNU HURD or Mach or Sys V/386 (such as ISC UNIX) */
	{0x82, "Linux swap"},	/* also Solaris */
	{0x83, "Linux native"},
	{0x85, "Linux extended"},
	{0x86, "NTFS volume set"},
	{0x87, "NTFS volume set"},
	{0x8e, "Linux LVM"},
	{0xc1, "DRDOS/sec (FAT-12)"},
	{0xc4, "DRDOS/sec (FAT-16 < 32M)"},
	{0xc6, "DRDOS/sec (FAT-16)"},
	{0xe1, "DOS access"},	/* DOS access or SpeedStor 12-bit FAT
		     	   extended partition */
	{0xe3, "DOS R/O"},		/* DOS R/O or SpeedStor */
	{0xeb, "BeOS fs"},
	{0xf2, "DOS secondary"},	/* DOS 3.3+ secondary */
	{0xfd, "Linux raid autodetect"},/* New (2.2.x) raid partition with
					       autodetect using persistent
					       superblock */
	{ 0, 0 }
};

struct dev_st {
	int ssize;
//	int part;
//	unsigned long length;
	unsigned char sectors;
	unsigned char heads;
//	unsigned short cylinders;
//	unsigned long start;
} hda_dev;

#define pt_offset(b, n)	((struct partition *)((b) + 0x1be + \
				(n) * sizeof(struct partition)))
#define cround(n)  (((n)/(dev->heads * dev->sectors))+1)

static unsigned int read4_little_endian(unsigned char *cp) 
{
	return (uint)(cp[0]) + ((uint)(cp[1]) << 8)
		+ ((uint)(cp[2]) << 16) + ((uint)(cp[3]) << 24);
}

unsigned int get_nr_sects(struct partition *p) 
{
	return (int)read4_little_endian(p->size4);
}

unsigned int get_start_sect(struct partition *p) 
{
	return read4_little_endian(p->start4);
}

static unsigned int get_partition_start(struct pte *pe)
{
	return pe->offset + get_start_sect(pe->part_table);
}

char *partition_type(unsigned char type)
{
	int i;

	for (i=0; i386_sys_types[i].name; i++)
		if (i386_sys_types[i].type == type)
			return (i386_sys_types[i].name);

	return 0;
}

void list_table(struct dev_st *dev)
{
	struct partition *p;
	char *type;
	int i, w;

	printk("    Start       End    Blocks   Id  System\n");

	for (i = 0 ; i < MAXIMUM_PARTS; i++) {
		struct pte *pe = &ptes[i];

		p = pe->part_table;
		if (p->sys_ind) {
			unsigned int psects = get_nr_sects(p);
			unsigned int pblocks = psects;
			unsigned int podd = 0;

			if (dev->ssize < 1024) {
				pblocks /= (1024 / dev->ssize);
				podd = psects % (1024 / dev->ssize);
			}
			if (dev->ssize > 1024)
				pblocks *= (dev->ssize / 1024);
                        printk( "%9ld %9ld %9ld%c  %2x  %s\n",
/* start */		    (long) cround(get_partition_start(pe)),
/* end */		    (long) cround(get_partition_start(pe) + psects
				- (psects ? 1 : 0)),
/* odd flag on end */	    (long) pblocks, podd ? '+' : ' ',
/* type id */		    p->sys_ind,
/* type name */		    (type = partition_type(p->sys_ind)) 
				? type : "Unknown"
			);
		}
	}
}

typedef enum {
	mechtype_caddy = 0,
	mechtype_tray  = 1,
	mechtype_popup = 2,
	mechtype_individual_changer = 4,
	mechtype_cartridge_changer  = 5
} mechtype_t;

struct mode_page_header {
	__u16 length;
	__u8 medium_type;
	__u8 reserved1;
	__u8 reserved2;
	__u8 reserved3;
	__u16 desc_length;
};

struct atapi_toc_header {
	unsigned short toc_length;
	byte first_track;
	byte last_track;
};

struct atapi_toc_msf {
	__u16  lba0;
	__u16  lba1;
};

struct atapi_toc_entry {
	byte reserved1;
	__u8 control : 4;
	__u8 adr     : 4;
	byte track;
	byte reserved2;
	struct atapi_toc_msf msf;
};

struct atapi_capabilities_page {
	struct mode_page_header header;
	__u8 page_code           : 6;
	__u8 reserved1           : 1;
	__u8 parameters_saveable : 1;

	byte     page_length;

	/* Drive supports read from CD-R discs (orange book, part II) */
	__u8 cd_r_read           : 1; /* reserved in 1.2 */
	/* Drive can read from CD-R/W (CD-E) discs (orange book, part III) */
	__u8 cd_rw_read          : 1; /* reserved in 1.2 */
	/* Drive supports reading CD-R discs with addressing method 2 */
	__u8 method2             : 1;
	/* Drive supports reading of DVD-ROM discs */
	__u8 dvd_rom             : 1;
	/* Drive supports reading of DVD-R discs */
	__u8 dvd_r_read          : 1;
	/* Drive supports reading of DVD-RAM discs */
	__u8 dvd_ram_read        : 1;
	__u8 reserved2		 : 2;

	/* Drive can write to CD-R discs (orange book, part II) */
	__u8 cd_r_write          : 1; /* reserved in 1.2 */
	/* Drive can write to CD-R/W (CD-E) discs (orange book, part III) */
	__u8 cd_rw_write	 : 1; /* reserved in 1.2 */
	/* Drive can fake writes */
	__u8 test_write          : 1;
	__u8 reserved3a          : 1;
	/* Drive can write DVD-R discs */
	__u8 dvd_r_write         : 1;
	/* Drive can write DVD-RAM discs */
	__u8 dvd_ram_write       : 1;
	__u8 reserved3           : 2;

	/* Drive supports audio play operations. */
	__u8 audio_play          : 1;
	/* Drive can deliver a composite audio/video data stream. */
	__u8 composite           : 1;
	/* Drive supports digital output on port 1. */
	__u8 digport1            : 1;
	/* Drive supports digital output on port 2. */
	__u8 digport2            : 1;
	/* Drive can read mode 2, form 1 (XA) data. */
	__u8 mode2_form1         : 1;
	/* Drive can read mode 2, form 2 data. */
	__u8 mode2_form2         : 1;
	/* Drive can read multisession discs. */
	__u8 multisession        : 1;
	__u8 reserved4           : 1;

	/* Drive can read Red Book audio data. */
	__u8 cdda                : 1;
	/* Drive can continue a read cdda operation from a loss of streaming.*/
	__u8 cdda_accurate       : 1;
	/* Subchannel reads can return combined R-W information. */
	__u8 rw_supported        : 1;
	/* R-W data will be returned deinterleaved and error corrected. */
	__u8 rw_corr             : 1;
	/* Drive supports C2 error pointers. */
	__u8 c2_pointers         : 1;
	/* Drive can return International Standard Recording Code info. */
	__u8 isrc                : 1;
	/* Drive can return Media Catalog Number (UPC) info. */
	__u8 upc                 : 1;
	__u8 reserved5           : 1;


	/* Drive can lock the door. */
	__u8 lock                : 1;
	/* Present state of door lock. */
	__u8 lock_state          : 1;
	/* State of prevent/allow jumper. */
	__u8 prevent_jumper      : 1;
	/* Drive can eject a disc or changer cartridge. */
	__u8 eject               : 1;
	__u8 reserved6           : 1;
	/* Drive mechanism types. */
	mechtype_t mechtype	 : 3;

	/* Audio level for each channel can be controlled independently. */
	__u8 separate_volume     : 1;
	/* Audio for each channel can be muted independently. */
	__u8 separate_mute       : 1;
	/* Changer can report exact contents of slots. */
	__u8 disc_present        : 1;  /* reserved in 1.2 */
	/* Drive supports software slot selection. */
	__u8 sss                 : 1;  /* reserved in 1.2 */
	__u8 reserved7           : 4;

	/* Note: the following four fields are returned in big-endian form. */
	/* Maximum speed (in kB/s). */
	unsigned short maxspeed;
	/* Number of discrete volume levels. */
	unsigned short n_vol_levels;
	/* Size of cache in drive, in kB. */
	unsigned short buffer_size;
	/* Current speed (in kB/s). */
	unsigned short curspeed;
	char pad[4];
};
