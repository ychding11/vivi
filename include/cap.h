/*
 * vivi capabilites
 * stored at offset 0x30 into the boot flash.
 */

/*
 * History
 *
 * 2002-4-19: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 */

/* +-------+-------+-------+-------+
 * | 8-bit | 8-bit | 8-bit | 8-bit |
 * +-------+-------+-------+-------+
 *     ^       ^       ^        ^
 *     |       |       |        |
 *     |       |       |        +- general
 *     |       |       +- reseved
 *     |       +- architecture(CPU) spcific
 *     +- hardware spcific
 */
#include "config.h"

#define VIVICAP_WAKEUP		(1 << 0) /* Support power management(especially wakeup) */
#define VIVICAP_PARTITIONS	(1 << 1) /* Suuport MTD partition management */
#define VIVICAP_MTD		(1 << 2) /* Support MTD management */
#define VIVICAP_PARAMS		(1 << 3) /* Support parameters for vivi */

#define VIVICAP_OS_SWITCH	(1 << 24) /* Support OS switching */
#define VIVICAP_LOAD_USR	(1 << 25) /* Support loading user image */

#define vivicap_canwakeup(caps)		((caps) & VIVICAP_WAKEUP)
#define vivicap_canpartitions(caps)	((caps) & VIVICAP_PARTITIONS)
#define vivicap_canmtd(caps)		((caps) & VIVICAP_MTD)
#define vivicap_canparams(caps)		((caps) & VIVICAP_PARAMS)

#define vivicap_canosswitch(caps)	((caps) & VIVICAP_OS_SWITCH)
#define vivicap_canloadusr(caps)	((caps) & VIVICAP_LOAD_USR)

#ifdef CONFIG_SA1100_KINGS
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_KINGS */
#ifdef CONFIG_SA1100_FORTE
#define VIVI_CAPS   (VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
             VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_FORTE */
#ifdef CONFIG_SA1100_SUNS
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_SUNS */
#ifdef CONFIG_SA1100_EXILIEN101
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_EXILIEN101 */
#ifdef CONFIG_SA1100_EXILIEN102
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_EXILIEN102 */
#ifdef CONFIG_S3C2400_SMDK 
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_S3C2400_SMDK */
#ifdef CONFIG_S3C2400_GAMEPARK
#ifdef CONFIG_OS_SWITCH_GP
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS | VIVICAP_OS_SWITCH)
#else
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS | VIVICAP_LOAD_USR)
#endif
#endif /* CONFIG_S3C2400_GAMEPARK */
#ifdef CONFIG_S3C2410_SMDK 
#define VIVI_CAPS	(VIVICAP_MTD)
#endif /* CONFIG_S3C2410_SMDK */
#ifdef CONFIG_SA1100_GILL
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_GILL */
#ifdef CONFIG_SA1100_ENDA
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_ENDA */
#ifdef CONFIG_SA1100_WISMO
#define VIVI_CAPS	(VIVICAP_WAKEUP | VIVICAP_PARTITIONS | VIVICAP_MTD | \
			 VIVICAP_PARAMS)
#endif /* CONFIG_SA1100_WISMO */
