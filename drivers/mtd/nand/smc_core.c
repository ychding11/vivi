/*
 * vivi/drivers/mtd/nand/smc_core.c: 
 *
 * Based on linux/drivers/mtd/nand/smc.c
 *
 * $Id: smc_core.c,v 1.3 2002/08/10 07:47:08 nandy Exp $
 *
 *
 * 잡담:
 *   왜 this->oobblock이라고 했을까? thsi->oobblcok_ofs이라고 하면
 *   좀더 명확하지 않을까?
 */

#include "config.h"
#include "mtd/mtd.h"
#include "mtd/nand.h"
#include "mtd/nand_ids.h"
#include "mtd/nand_ecc.h"
#include "time.h"
#include "printk.h"
#include <types.h>
#include <errno.h>

/*
 * Macros for low-level register control
 */
#define nand_select()	this->hwcontrol(NAND_CTL_SETNCE); \
			nand_command(mtd, NAND_CMD_RESET, -1, -1); \
			udelay(10);
#define nand_deselect()	this->hwcontrol(NAND_CTL_CLRNCE);

static inline void 
sm_swap(u_char *x, u_char *y) {
	u_char tmp;
	tmp = *x;
	*x = *y;
	*y = tmp;
}

/* define if you'll be using < 2M SMC device */
#undef USE_256BYTE_NAND_FLASH

/*
 * Send command to NAND device
 */
static void
nand_command(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
	register struct nand_chip *this = mtd->priv;

	/* Begin command latch cycle */
	this->hwcontrol(NAND_CTL_SETCLE);

	this->hwcontrol(NAND_CTL_DAT_OUT);

	/*
	 * Write out the command to the device.
	 */
	if (command != NAND_CMD_SEQIN)
		this->write_cmd(command);
	else {
		if (mtd->oobblock == 256 && column >= 256) {
			column -= 256;
			this->write_cmd(NAND_CMD_RESET);
			this->write_cmd(NAND_CMD_READOOB);
			this->write_cmd(NAND_CMD_SEQIN);
		} else if (mtd->oobblock == 512 && column >= 256) {
			if (column < 512) {
				column -= 256;
				this->write_cmd(NAND_CMD_READ1);
				this->write_cmd(NAND_CMD_SEQIN);
			} else {
				column -= 512;
				this->write_cmd(NAND_CMD_READOOB);
				this->write_cmd(NAND_CMD_SEQIN);
			}
		} else {
			this->write_cmd(NAND_CMD_READ0);
			this->write_cmd(NAND_CMD_SEQIN);
		}
	}

	/* Set ALE and clear CLE to start address cycle */
	this->hwcontrol(NAND_CTL_CLRCLE);
	this->hwcontrol(NAND_CTL_SETALE);

	/* Serially input address */
	if (column != -1)
		this->write_addr(column);
	if (page_addr != -1) {
		this->write_addr((u_char)(page_addr & 0xff));
		this->write_addr((u_char)((page_addr >> 8) & 0xff));
		/* One more address cycle for higher density devices */
		if (mtd->size & 0x0c000000) {
			this->write_addr((u_char)((page_addr >> 16) & 0xff));
		}
	}

	/* Latch in address */
	this->hwcontrol(NAND_CTL_CLRALE);
	
	this->hwcontrol(NAND_CTL_DAT_IN);
	/* Pause for 15us */
	udelay(15);
 }

/*
 * NAND read
 */
static int
nand_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	int j, col, page, state;
	struct nand_chip *this = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__"(): from = 0x%08lx, len = %i\n",
		(unsigned int)from, (int)len);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	/* First we calculate the starting page */
	page = from >> this->page_shift;

	/* Get raw starting column */
	col = from & (mtd->oobblock - 1);

	/* State machine for devices having pages larger than 256 bytes */
	state = (col < mtd->eccsize) ? 0 : 1;

	/* Calculate column address within ECC block context */
	col = (col >= mtd->eccsize) ? (col - mtd->eccsize) : col;

	/* Initialize return value */
	*retlen = 0;

	/* Select the NAND device */
	nand_select();

	/* Loop until all data read */
	while (*retlen < len) {
		/* Send the read command */
		if (!state)
			nand_command(mtd, NAND_CMD_READ0, col, page);
		else
			nand_command(mtd, NAND_CMD_READ1, col, page);

		this->wait_for_ready();

		/* Read the data directly into the return buffer */
		if ((*retlen + (mtd->eccsize - col)) >= len) {
			while (*retlen < len)
				buf[(*retlen)++] = this->read_data();
			/* We're done */
			continue;
		} else {
			for (j = col; j < mtd->eccsize; j++)
				buf[(*retlen)++] = this->read_data();
		}

		/*
		 * If the amount of data to be read is greater than
		 * (256 - col), then all subsequent reads will take
		 * place on page or half-page (in the case of 512 byte
		 * page devices) aligned boundaries and the column
		 * address will be zero. Setting the column address to
		 * zero after the first read allows us to simplify
		 * the reading of data and the if/else statements above.
		 */
		if (col)
			col = 0x00;

		/* Increment page address */
		if ((mtd->oobblock == 256) || state)
			page++;

		/* Toggle state machine */
		if (mtd->oobblock == 512)
			state = state ? 0 : 1;
	}

	/* De-select the NAND device */
	nand_deselect();

	/* Return happy */
	return 0;	
}

/*
 * if mtd->oobblock == 512
 */
inline int
smc_read_ecc_512(struct mtd_info *mtd, u_char *ecc_code)
{
	struct nand_chip *this = mtd->priv;
	u_char ecc_calc[3];
	int j, ret;

	/* Read in a block big enough for ECC */
	for (j = 0; j < (mtd->oobblock + mtd->oobsize); j++)
		this->data_buf[j] = this->read_data();

	for (j = 0; j < mtd->oobsize; j++)
		ecc_code[j] = this->data_buf[(mtd->oobblock + j)];

	nand_calculate_ecc(&this->data_buf[0], &ecc_calc[0]);
	sm_swap(&ecc_calc[0], &ecc_calc[1]);
	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__"(): ECC [%02x%02x%02x : %02x%02x%02x]\n",
		ecc_code[SMC_OOB_ECC1], ecc_code[SMC_OOB_ECC1+1],
		ecc_code[SMC_OOB_ECC1+2], ecc_calc[0], ecc_calc[1], ecc_calc[2]);
	ret = nand_correct_data(&this->data_buf[0],
				&(ecc_code[SMC_OOB_ECC1]), &ecc_calc[0]);
	if (ret == -1)
		return ret;

	nand_calculate_ecc(&this->data_buf[mtd->eccsize], &ecc_calc[0]);
	sm_swap(&ecc_calc[0], &ecc_calc[1]);
	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__"(): ECC [%02x%02x%02x : %02x%02x%02x]\n",
		ecc_code[SMC_OOB_ECC2], ecc_code[SMC_OOB_ECC2+1],
		ecc_code[SMC_OOB_ECC2+2], ecc_calc[0], ecc_calc[1], ecc_calc[2]);
	ret = nand_correct_data(&this->data_buf[mtd->eccsize],
				&(ecc_code[SMC_OOB_ECC2]), &ecc_calc[0]);
	if (ret == -1)
		return ret;

	return 0;
}

/*
 * NAND read with ECC
 */
static int
nand_read_ecc(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,
              u_char *buf, u_char *ecc_code)
{
	int j, offset, page;
	struct nand_chip *this = mtd->priv;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__ "(): from = 0x%08x, len = %i\n", (unsigned int)from,
		(int)len);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__ "(): Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	/* Select the NAND device */
	nand_select();

	/* Initialize return value */
	*retlen = 0;

#ifdef USE_256BYTE_NAND_FLASH
	/* First we calculate the starting page */
	page = from >> this->page_shift;

	/* Get raw starting column */
	offset = from & (mtd->oobblock - 1);

	/* if the length of Page is 256bytes,
	 * 2 Pages must be taken for 1 Sector and as a result,
	 * higher level 8 bytes of information
	 * among the above 16 byte information must coincide with
	 * Spare(or OOB) of Even-Page while lowel level 8 bytes
	 * coincide with Spar(or OOB) of Odd-page.
	 *   i.e, [0 block][oob][1 block][oob]
	 *        [2 block][oob][3 block][oob]...
	 */
	if (mtd->oobblock == 256) {
		u_char ecc_calc[3];
		int oob_offset;

		/* Loop until all data read */
		while (*retlen < len) {
			nand_command(mtd, NAND_CMD_READ0, 0x00, page);

			this->wait_for_ready();

			/* Read in a block big enough for ECC */
			for (j = 0; j < mtd->eccsize; j++)
				this->data_buf[j] = this->read_data();

			if (!(page & 0x1)) {	/* page is odd! */
				nand_command(mtd, NAND_CMD_READOOB,
					     SMC_OOB256_ECC1, page + 1);
				oob_offset = SMC_OOB_ECC1;
			} else {
				nand_command(mtd, NAND_CMD_READOOB,
					     SMC_OOB256_ECC2, page);
				oob_offset = SMC_OOB_ECC2;
			}

			this->wait_for_ready();

			for (j = 0; j < 3; j++)
				ecc_code[oob_offset + j] = this->read_data();
			nand_calculate_ecc(&this->data_buf[0], &ecc_calc[0]);
			sm_swap(&ecc_calc[0], &ecc_calc[1]);
			ret = nand_correct_data(&this->data_buf[0], 
						&(ecc_code[oob_offset]), 
						&ecc_calc[0]);
			if (ret == -1)
				goto nand_read_ecc_err;

			/* Read the data from ECC data buffer into return buffer */
			if ((*retlen + (mtd->eccsize - offset)) > = len) {
				while (*retlen < len)
					buf[(*retlen)++] = this->data_buf[offset++];
				/* We're done */
				continue;
			} else {
				for (j = offset; j < mtd->eccsize; j++)
					buf[(*retlen)++] = this->data_buf[j];
			}

			/*
			 * If the amount of data to be read is greater than
			 * (256 - offset), then all subsequent reads will take
			 * place on page or half-page (in the case of 512 byte
			 * page devices) aligned boundaries and the column
			 * address will be zero. Setting the column address to
			 * to zero after the first read allows us to simplify
			 * the reading of data and the if/else statements above.
			 */
			if (offset)
				offset = 0x00;

			/* Increment page address */
			page++;
		}
	} 
	else 
#endif	/* USE_256BYTE_NAND_FLASH */
	{ /* mtd->oobblock == 512 */
		size_t last, next, len2;

		last = from + len;
		for (next = from; from < last; ) {
			page = from >> this->page_shift;
			offset = from & (mtd->oobblock - 1);
			len2 = mtd->oobblock - offset;
			next += len2;
			nand_command(mtd, NAND_CMD_READ0, 0x00, page);

			this->wait_for_ready();

			ret = smc_read_ecc_512(mtd, ecc_code);
			if (ret == -1)
				goto nand_read_ecc_err;

			if (next >= last)
				if ((last & (mtd->oobblock - 1)) != 0)
					len2 = (last & (mtd->oobblock - 1)) - offset;

			for (j = 0; j < len2; j++)
				buf[(*retlen) + j] = this->data_buf[offset + j];
			*retlen += len2;
			from = next;
		}
	}

	ret = 0;

	/* De-select the NAND device */
	nand_deselect();

	return ret;

nand_read_ecc_err:
	DEBUG(MTD_DEBUG_LEVEL0,
		__FUNCTION__"(): Failed ECC read, page 0x%08lx\n", page);
	return -EIO;
}

/*
 * NAND read out-of-band
 */
static int
nand_read_oob(struct mtd_info *mtd, loff_t from, size_t len, 
              size_t *retlen, u_char *buf)
{
	int i, offset, page;
	struct nand_chip *this = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__ "(): from = 0x%08x, len = %i\n", (unsigned int)from,
		(int)len);

	/* Shift to get page */
	page = ((int)from) >> this->page_shift;

	/* Mask to get column */
	offset = from & 0x0f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow read past end of page */
	if ((offset + len) > mtd->oobsize) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Attempt read past end of page " \
			"0x%08lx, column %i, length %i\n", page, offset, len);
		return -EINVAL;
	}

	/* Select the NAND device */
	nand_select();

	/* Send the read command */
	nand_command(mtd, NAND_CMD_READOOB, offset, page);

	this->wait_for_ready();

	/* Read the data */
	for (i = 0; i < len; i++)
		buf[i] = this->read_data();

	/* De-select the NAND device */
	nand_deselect();

	/* Return happy */
	*retlen = len;
	return 0;
}

/*
 * NAND write
 */
static int
nand_write(struct mtd_info *mtd, loff_t to, size_t len, 
           size_t *retlen, const u_char *buf)
{
	int i, page, col, cnt, status;
	struct nand_chip *this = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__"(): to = 0x%08x, len = %i\n", (unsigned int)to,
		(int) len);

	/* Do not allow write past end of page */
	if ((to + len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Attempted write past end of device\n");
		return -EINVAL;
	}

	/* Shift to get page */
	page = ((int)to) >> this->page_shift;

	/* Get the starting column */
	col = to & (mtd->oobblock - 1);

	/* Initialize return length value */
	*retlen = 0;

	/* Select the NAND device */
	nand_select();

	/* Check the WP bit */
	nand_command(mtd, NAND_CMD_STATUS, -1, -1);

	this->wait_for_ready();

	if (!(this->read_data() & SMC_STAT_NOT_WP)) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Device is write protected!!!\n");
		i = -EPERM;
		goto nand_write_exit;
	}

	/* Loop until all data is written */
	while (*retlen < len) {
		/* Write data into buffer */
		if ((col + len) >= mtd->oobblock)
			for (i = col, cnt = 0; i < mtd->oobblock; i++, cnt++)
				this->data_buf[i] = buf[(*retlen + cnt)];
		else
			for (i = col, cnt = 0; cnt < (len - *retlen); i++, cnt++)
				this->data_buf[i] = buf[(*retlen + cnt)];
		/* Write ones for partial page programming */
		for (i = mtd->oobblock; i < (mtd->oobblock + mtd->oobsize); i++)
			this->data_buf[i] = 0xff;
		
		/* Write pre-padding bytes into buffer */
		for (i = 0; i < col; i++)
			this->data_buf[i] = 0xff;

		/* Write post-padding bytes into buffer */
		if ((col + (len - *retlen)) < mtd->oobblock) {
			for (i = (col + cnt); i < mtd->oobblock; i++)
				this->data_buf[i] = 0xff;
		}

		/* Send command to begin auto page programming */
		nand_command(mtd, NAND_CMD_SEQIN, 0x00, page);

		/* Write out complete page of data */
		this->hwcontrol(NAND_CTL_DAT_OUT);
		for (i = 0; i < (mtd->oobblock + mtd->oobsize); i++)
			this->write_data(this->data_buf[i]);
		this->hwcontrol(NAND_CTL_DAT_IN);

		/* Send command to actually program the data */
		nand_command(mtd, NAND_CMD_PAGEPROG, -1, -1);

		this->wait_for_ready();

		/*
		 * Wait for program operation to complete. This could
		 * take up to 3000us (3ms) on some devices. so we try
		 * and exit as quickly as possible.
		 */
		status = 0;
		for (i = 0; i < 24; i++) {
			/* Delay for 125us */
			udelay(125);

			/* Check for status */
			nand_command(mtd, NAND_CMD_STATUS, -1, -1);
			status = (int)this->read_data();
			if (status & SMC_STAT_READY)
				break;
		}

		/* See if device thinks it succeeded */
		if (status & SMC_STAT_WRITE_ERR) {
			DEBUG(MTD_DEBUG_LEVEL0,
				__FUNCTION__"(): Failed write, page 0x%08x, " \
				"%6i bytes were sucesful\n", page, *retlen);
			i = -EIO;
			goto nand_write_exit;
		}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
		/*
		 * The NAND device assumes that it is always writing to
		 * a cleanly erased page. Hence, it performs its internal
		 * write verification only on bits that transitioned from
		 * 1 to 0. The device does NOT verify the whole page on
		 * a byte by byte basis. It is possible that the page was
		 * not completely erased or the page is becoming unusable
		 * due to wear. The read with ECC would catch the error
		 * later when the ECC page check fails, but we would rather
		 * catch it early in the page write stage. Better to write
		 * no data than invalid data.
		 */

		/* Send command to read back the page */
		if (col < mtd->eccsize)
			nand_command(mtd, NAND_CMD_READ0, col, page);
		else
			nand_command(mtd, NAND_CMD_READ1, col - 256, page);

		this->wait_for_ready();

		/* Loop through and verify the data */
		for (i = col; i < cnt; i++) {
			if (this->data_buf[i] != this->read_data()) {
				DEBUG(MTD_DEBUG_LEVEL0,
					__FUNCTION__"(): Failed write verify, " \
					"page 0x%08x, %6i bytes were succesful\n",
					page, *retlen);
				i = -EIO;
				goto nand_write_exit;
			}
		}
#endif

		/* 
		 * If we are writing a large amount of data and/or it
		 * crosses page or half-page boundaries, we set the
		 * the column to zero. It simplifies the program logic.
		 */
		if (col)
			col = 0x00;

		/* Update written bytes count */
		*retlen += cnt;

		/* Increment page address */
		page++;
		
	}

	/* Return happy */
	*retlen = len;
	i = 0;

nand_write_exit:
	/* De-select the NAND device */
	nand_deselect();

	return i;
}

/*
 * NAND write witdh ECC, but only 1 sector!
 */
static int
nand_write_ecc(struct mtd_info *mtd, loff_t to, size_t len,
               size_t *retlen, const u_char *buf, u_char *ecc_code)
{
	int i, page, cnt, status, ret;
	struct nand_chip *this = mtd->priv;
	unsigned int sector_size, page_size, oob_size;

	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__"(): to = 0x%08x, len = %i\n", (unsigned int)to,
		(int)len);

	sector_size = this->dev->szS;
	page_size = mtd->oobblock;
	oob_size = mtd->oobsize;
	if (to & (sector_size - 1)) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Not Sector aligned\n");
		return -EINVAL;
	}
	if (len != sector_size) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Only 1 Sector\n");
		return -EINVAL;
	}
	/* Do not allow write past end of page */
	if ((to + len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Attempted write past end of device\n");
		return -EINVAL;
	}

	/* Shift to get page */
	page = ((int) to) >> this->page_shift;

	/* Initialize return length value */
	*retlen = 0;

	/* Select the NAND device */
	nand_select();

	/* Check the WP bit */
	nand_command(mtd, NAND_CMD_STATUS, -1, -1);

	this->wait_for_ready();

	if (!(this->read_data() & SMC_STAT_NOT_WP)) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Device is write protected!!!\n");
		ret = -EPERM;
		goto nand_write_err;
	}

	/* Loop until all data is written */
	while (*retlen < len) {
		/* Send command to begin auto page programming */
		nand_command(mtd, NAND_CMD_SEQIN, 0x00, page);

		this->hwcontrol(NAND_CTL_DAT_OUT);

		/* Write out complete page of data */
		for (i = 0, cnt = 0; i < page_size; i++, cnt++)
			this->write_data(buf[(*retlen) + cnt]);

		/* Write ones for partial page programming */
		for (i = 0; i < oob_size; i++) {
#ifdef USE_256BYTE_NAND_FLASH
			if (*retlen & (sector_size -1))
				this->write_data(ecc_code[SMC_OOB256_SIZE + i]);
			else
#endif
				this->write_data(ecc_code[i]);
		}

		this->hwcontrol(NAND_CTL_DAT_IN);

		/* Send command to actually program the data */
		nand_command(mtd, NAND_CMD_PAGEPROG, -1, -1);

		this->wait_for_ready();

		/*
		 * Wait for program operation to complete. This could
		 * take up to 3000us (3ms) on some devices, so we try
		 * and exit as quickly as possible
		 */
		status = 0;
		for (i = 0; i < 25; i++) {
			/* Delay for 125us */
			udelay(125);		    

			/* Check the status */
			nand_command(mtd, NAND_CMD_STATUS, -1, -1);
			status = (int)this->read_data();
			if (status & SMC_STAT_READY)
				break;
		}

		/* See if device thins it succeeded */
		if (status & SMC_STAT_WRITE_ERR) {
			DEBUG(MTD_DEBUG_LEVEL0,
				__FUNCTION__"(): Failed write, page 0x%08x, " \
				"%6i bytes were succesful\n", page, *retlen);
			ret = -EIO;
			goto nand_write_err;
		}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
		/*
		 * The NAND device assumes that it is always writing to
		 * a cleanly erased page. Hence, it performs its internal
		 * write verification only on bits tha transitioned from
		 * 1 to 0. The device does NOT verify the whole page on a
		 * byte by byte basis. It is possible that the page was
		 * not completely erased or the page is becoming unusable
		 * due to wear. The read with ECC would catch the error
		 * later when the ECC page check fails, but we would rather
		 * catch it early in the page write stage. Better to write
		 * no data than invalid data.
		 */

		/* Send command to read back the page */
#ifdef USE_256BYTE_NAND_FLASH
		if (*retlen & (sector_size - 1))
			nand_command(mtd, NAND_CMD_READ0, 0x00, page + 1);
		else
#endif
			nand_command(mtd, NAND_CMD_READ0, 0x00, page);

		this->wait_for_ready();

		/* Loop through and verify the data */
		for (i = 0; i < page_size; i++) {
			if (this->data_buf[i] != this->read_data()) {
				DEBUG(MTD_DEBUG_LEVEL0,
					__FUNCTION__"(): Failed write verify, " \
					"page 0x%08x, %6i bytes were succesful\n",
					page, *retlen);
				ret = -EIO;
				goto nand_write_err;
			}
		}
#endif
		/* Update written bytes count */
		*retlen += cnt;

		/* Increment page address */
		page++;
	}

	*retlen = len;
	ret = 0;

nand_write_err:
	/* De-select the NAND device */
	nand_deselect();

	return ret;
}

/*
 * NAND write out-of-band
 */
static int
nand_write_oob(struct mtd_info *mtd, loff_t to, size_t len,
               size_t *retlen, const u_char *buf)
{
	int i, offset, page, status, ret;
	struct nand_chip *this = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__"(): to = 0x%08x, len = %i\n", (unsigned int)to,
		(int)len);

	/* Shift to get page */
	page = ((int)to) >> this->page_shift;

	/* Mask to get column */
	offset = to & 0x1f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow write past end of page */
	if ((offset + len) > mtd->oobsize) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* Select the NAND device */
	nand_select();

	/* Check the WP bit */
	nand_command(mtd, NAND_CMD_STATUS, -1, -1);

	this->wait_for_ready();

	if (!(this->read_data() & SMC_STAT_NOT_WP)) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Device is write protected!!!\n");
		ret = -EPERM;
		goto nand_write_oob_err;
	}

	/* Write out desired data */
	nand_command(mtd, NAND_CMD_SEQIN, offset + mtd->oobblock, page);
	this->hwcontrol(NAND_CTL_DAT_OUT);
	for (i = 0; i < len; i++)
		this->write_data(buf[i]);
	this->hwcontrol(NAND_CTL_DAT_IN);

	/* Send command to program the OOB data */
	nand_command(mtd, NAND_CMD_PAGEPROG, -1, -1);

	this->wait_for_ready();

	/*
	 * Wait for program operation to complete. This could
	 * take up to 3000us (3ms) on some devices, so we try
	 * and exit as quickly as possible.
	 */
	status = 0;
	for (i = 0; i < 24; i++) {
		/* Delay for 125us */
		udelay(125);

		/* Check the status */
		nand_command(mtd, NAND_CMD_STATUS, -1, -1);

		this->wait_for_ready();

		status = (int)this->read_data();
		if (status & SMC_STAT_READY)
			break;
	}

	/* See if device thinks it succeeded */
	if (status & SMC_STAT_WRITE_ERR) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Failed write, page 0x%08x\n", page);
		ret = -EIO;
		goto nand_write_oob_err;
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/* Send command to read back the data */
	nand_command(mtd, NAND_CMD_READOOB, offset, page);

	this->wait_for_ready();

	/* Loop through and verify the data */
	for (i = 0; i < len; i++) {
		if (buf[i] != this->read_data()) {
			DEBUG(MTD_DEBUG_LEVEL0,
				__FUNCTION__"(): Failed write verify, page 0x%08x\n",
				page);
			ret = -EIO;
			goto nand_write_oob_err;
		}
	}
#endif

	/* Return happy */
	*retlen = len;
	ret = 0;
	
nand_write_oob_err:
	/* De-select the NAND device */
	nand_deselect();

	return ret;
}

/*
 * NAND erase a block
 */
static int
nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int i, page, len, status, pages_per_block;
	struct nand_chip *this = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3,
		__FUNCTION__"(): start = 0x%08x, len = %i\n",
		(unsigned int)instr->addr, (unsigned int)instr->len);

	/* Start address must aligned on block boundary */
	if (instr->addr & (mtd->erasesize - 1)) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Unaligned address\n");
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (instr->len & (mtd->erasesize - 1)) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Length not block aligned\n");
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->len + instr->addr) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Erase past end of device\n");
		return -EINVAL;
	}

	/* Shift to get first page */
	page = (int)(instr->addr >> this->page_shift);

	/* Calculate pages in each block */
	pages_per_block = mtd->erasesize / mtd->oobblock;

	/* Select the NAND device */
	nand_select();

	/* Check the WP bit */
	nand_command(mtd, NAND_CMD_STATUS, -1, -1);

	this->wait_for_ready();

	if (!(this->read_data() & SMC_STAT_NOT_WP)) {
		DEBUG(MTD_DEBUG_LEVEL0,
			__FUNCTION__"(): Device is write protected!!!\n");
		nand_deselect();
		return -EIO;
	}

	/* Loop through the pages */
	len = instr->len;
	while (len) {
		/* Send commands to erase a page */
		nand_command(mtd, NAND_CMD_ERASE1, -1, page);
		nand_command(mtd, NAND_CMD_ERASE2, -1, -1);

		this->wait_for_ready();

		/*
		 * Wait for program operation to complete. This could
		 * take up to 4000us (4ms) on some devices, so we try
		 * and exit as quickly as possible.
		 */
		status = 0;
		for (i = 0; i < 32; i++) {
			/* Delay for 125us */
			udelay(125);

			/* Check the status */
			nand_command(mtd, NAND_CMD_STATUS, -1, -1);

			this->wait_for_ready();

			status = (int)this->read_data();
			if (status & SMC_STAT_READY)
				break;
		}

		/* See if block erase succeeded */
		if (status & SMC_STAT_WRITE_ERR) {
			DEBUG(MTD_DEBUG_LEVEL0,
				__FUNCTION__"(): Failed erase, page 0x%08x\n", page);
			nand_deselect();
#if 0
			instr->state = MTD_ERASE_FAILED;
			if (instr->callback)
				instr->callback(instr);
#endif
			return -EIO;
		}

		/* Increment page address and decrement length */
		len -= mtd->erasesize;
		page += pages_per_block;
	}

	/* De-select the NAND device */
	nand_deselect();

#if 0
	/* Do call back function */
	instr->state = MTD_ERASE_DONE;
	if (instr->callback)
		instr->callback(instr);
#endif

	/* Return happy */
	return 0;	
}

/*
 * Scan for the SMC device
 */
int
smc_scan(struct mtd_info *mtd)
{
	int i, nand_maf_id, nand_dev_id;
	struct nand_chip *this = mtd->priv;

	/* Select the device */
	nand_select();

	/* Send the command for reading device ID */
	nand_command(mtd, NAND_CMD_READID, 0x00, -1);

	this->wait_for_ready();

	/* Read manufacturer and device IDs */
	nand_maf_id = this->read_data();
	nand_dev_id = this->read_data();

	/* Print and sotre flash device information */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {
		if (nand_maf_id == nand_flash_ids[i].manufacture_id &&
		    nand_dev_id == nand_flash_ids[i].model_id) {
#ifdef USE_256BYTE_NAND_FLASH
			if (!mtd->size) {
				mtd->name = nand_flash_ids[i].name;
				mtd->erasesize = nand_flash_ids[i].erasesize;
				mtd->size = (1 << nand_flash_ids[i].chipshift);
				mtd->eccsize = 256;
				if (nand_flash_ids[i].page256) {
					mtd->oobblock = 256;
					mtd->oobsize = 8;
					this->page_shift = 8;
				} else {
					mtd->oobblock = 512;
					mtd->oobsize = 16;
					this->page_shift = 9;
				}
				this->dev = &nand_smc_info[GET_DI_NUM(nand_flash_ids[i].chipshift)];
			}
#else
			if (!(mtd->size) && !(nand_flash_ids[i].page256)) {
				mtd->name = nand_flash_ids[i].name;
				mtd->erasesize = nand_flash_ids[i].erasesize;
				mtd->size = (1 << nand_flash_ids[i].chipshift);
				mtd->eccsize = 256;
				mtd->oobblock = 512;
				mtd->oobsize = 16;
				this->page_shift = 9;
				this->dev = &nand_smc_info[GET_DI_NUM(nand_flash_ids[i].chipshift)];
			}
#endif
			printk("NAND device: Manufacture ID:" \
				" 0x%02x, Chip ID: 0x%02x (%s)\n",
				nand_maf_id, nand_dev_id, mtd->name);
			break;
		}	    
	}

	/* De-select the device */
	nand_deselect();

	/* Print warning message for no device */
	if (!mtd->size) {
		printk("No NAND device found!!!\n");
		return 1;
	}

	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH | MTD_ECC;
	mtd->module = NULL;
	mtd->ecctype = MTD_ECC_SW;
	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->read_ecc = nand_read_ecc;
	mtd->write_ecc = nand_write_ecc;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->lock = NULL;
	mtd->unlock = NULL;

	/* Return happy */
	return 0;
}
