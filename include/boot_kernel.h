#ifndef _VIVI_BOOT_KERNEL_H_
#define _VIVI_BOOT_KERNEL_H_

/* 
 * Media Type: A type of storage device that contains the linux kernel
 *
 *  +----------------+-----------------------------------------+
 *  | Value(Integer) |  Type                                   |
 *  +----------------+-----------------------------------------+
 *  |       0        |  UNKNOWN                                | 
 *  |       1        |  RAM                                    |
 *  |       2        |  NOR Flash Memory                       |
 *  |       3        |  SMC (NAND Flash Memory) on the S3C2410 |
 *  +----------------+-----------------------------------------+
 */
enum {
	MT_UNKNOWN = 0,
	MT_RAM,
	MT_NOR_FLASH,
	MT_SMC_S3C2410
};

#endif /* _VIVI_BOOT_KERNEL_H_ */
