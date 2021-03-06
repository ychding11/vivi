/*
   md5.h - Declaration of functions and data types used for MD5 sum
   computing library functions.
   Copyright (C) 1995, 1996, 1999 Free Software Foundation, Inc.
   NOTE: The canonical source of this file is maintained with th GNU C
   Library.  Bugs can be reported to bug-glibc@prep.ai.mit.edu.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
*/

/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995. */
/* Hacked to work with vivi by Janghoon Lyu <nandy@mizi.com>, 2002.  */

#ifndef _MD5_H_
#define _MD5_H_

#include <config.h>
#include <types.h>

/* unsigned 32-bit data type */
typedef unsigned long md5_uint32;

/* Structure to save state of computation between the single steps. */
struct md5_ctx {
	md5_uint32 A;
	md5_uint32 B;
	md5_uint32 C;
	md5_uint32 D;

	md5_uint32 total[2];
	md5_uint32 buflen;
	char buffer[128];
};

/*
 * The following three functions are build up the low level used in
 * the functions `md5_stream' and `md5_buffer'.
 */

/*
 * Initialize structure containing state of computation.
 * (RFC 1321, 3.3: Step 3)
 */
extern void md5_init_ctx(struct md5_ctx *ctx);
/*
 * Starting with the result of former calls of this function (or the
 * initialization function update the context for the next LEN bytes
 * starting at BUFFER.
 * It is necessary that LEN is a multiple of 64!!! 
 */ 
extern void md5_process_block(const void *buffer, size_t len, struct md5_ctx *ctx);
/*
 * Starting with the result of former calls of this function (or the
 * initializaton function update the context for the next LEN bytes
 * starting at BUFFER.
 * It is NOT required that LEN is a multiple of 64.
 */
extern void md5_process_bytes(const void *buffer, size_t len, struct md5_ctx *ctx);

/*
 * Process the remaining bytes in the buffer and put result from CTX
 * in first 16 bytes following RESBUF. The result is always in little
 * endian byte order, so that a byte-wise output yields to the wanted
 * ASCII representation of the message digest.
 *
 * IMPORTANT: On some systems it is requires that RESBUF be correctly
 * aligned for a 32 bit value.
 */
extern void *md5_finish_ctx(struct md5_ctx *ctx, void *resbuf);

/*
 * Put result from CTX in first 16 bytes following RESBUF. The result is
 * always in little indian byte order, so that a byte-wise output yields
 * to the wanted ASCII representation of the message digest.
 *
 * IMPORTANT: On some systems it is required that RESBUF is correctly
 * aligned for a 32 bits value. 
 */
extern void *md5_read_ctx(const struct md5_ctx *ctx, void *resbuf);

/*
 * Compute MD5 message digest for bytes read from STREAM. The
 * resulting message digest number will be written into the 16 bytes
 * beginning at RESBLOCK.
 */
/*extern int md5_stream(FILE *stream, void *resblock);*/

/*
 * Compute MD5 message digest for LEN bytes beginning at BUFFER. The
 * result is always in little indian byte order, so that a byte-wise
 * output yields to the wanted ASCII representation of the message
 * digest.
 */
extern void *md5_buffer(const char *buffer, size_t len, void *resblock);

#define rol(x, n)	(((x) << (n)) | ((x) >> (32-(n))))

/* interfaces for vivi */
#ifdef CONFIG_MD5
void check_md5sum(const char *buffer, size_t len, char *file_name);
#else
#define check_md5sum(a, b, c)
#endif

#endif /* _MD5_H_ */
