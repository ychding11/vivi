/* $Id: priv_data.h,v 1.3 2002/10/22 08:07:17 nandy Exp $ */

#ifndef __VIVI_PRIV_DATA_H__
#define __VIVI_PRIV_DATA_H__

#include <types.h>

#define MAX_PART_NAME		16
#define MAX_PARAM_NAME		24

#define MF_LOCKED		0x00000001
#define MF_MUSTERASE		0x00000002
#define MF_BONFS		0x00000004
#define MF_JFFS2		0x00000008
#define MF_ISRAM		0x00000010

#define UNKNOWN_MODEM		0
#define X_MODEM			1
#define Y_MODEM			2
#define Z_MODEM			3

typedef struct mtd_partiton {
	char name[MAX_PART_NAME];	/* partition name */
	ulong offset;
	ulong size;
	ulong flag;
} mtd_partition_t;

typedef unsigned long param_value_t;

typedef struct parameter {
	char name[MAX_PARAM_NAME];
	param_value_t value;
	void (*update_func)(param_value_t value);
} vivi_parameter_t;	

extern const char vivi_param_magic[8];
extern const char linux_cmd_magic[8];
extern const char mtd_part_magic[8];

extern mtd_partition_t default_mtd_partitions[];
extern int default_nb_part;
extern vivi_parameter_t default_vivi_parameters[];
extern int default_nb_params;
extern char linux_cmd[];

extern int *nb_mtd_parts;
extern int *nb_params;

/* API */
int get_default_mtd_partition(void);
int get_default_param_tlb(void);
int get_default_linux_cmd(void);
mtd_partition_t *get_mtd_partition(const char *);
mtd_partition_t *find_mtd_partition(ulong ofs);
vivi_parameter_t *get_param(const char*);
param_value_t get_param_value(const char *, int *);
int set_param_value(const char *name, param_value_t value);
char *get_linux_cmd_line(void);
void display_param_tlb(void);
void display_mtd_partition(void);
int save_priv_data_blk(void);
int init_priv_data(void);

#endif /* __VIVI_PRIV_DATA_H__ */
