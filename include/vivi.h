/*
 * Refer to bootldr/bootldr.h (C) Compaq Computer Corporation.
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * History
 *
 * 2001-10-04: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2002-01-26: Janghoon Lyu <nandy@mizi.com>
 *    - Cleaned up
 */


#ifndef _BOOTCONFIG_H_
#define _BOOTCONFIG_H_

#include "config.h"

/*
 * Magic numbers
 */
#define VIVI_MAGIC		0x646c7462	/* btld: marks a valid vivi image */
#define ELF_MAGIC		(('E' << 24) | ('L' << 16) | ('F' << 8) | 0x7f)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif /* _BOOTCONFIG_H_ */
