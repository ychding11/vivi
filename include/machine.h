#ifndef _VIVI_MACHINE_H_
#define _VIVI_MACHINE_H_

#include "config.h"

#ifdef CONFIG_SA1100_KINGS
#include "platform/kings.h"
#endif
#ifdef CONFIG_SA1100_FORTE
#include "platform/forte.h"
#endif
#ifdef CONFIG_SA1100_EXILIEN102
#include "platform/exilien102.h"
#endif
#ifdef CONFIG_S3C2400_GAMEPARK
#include "platform/gamepark.h"
#endif
#ifdef CONFIG_S3C2410_SMDK
#include "platform/smdk2410.h"
#endif
//add by threewater
#ifdef CONFIG_S3C2410_MP3
#include "platform/2410-mp3.h"
#endif
#ifdef CONFIG_SA1100_SUNS
#include "platform/suns.h"
#endif
#ifdef CONFIG_SA1100_SUNS_OLD
#include "platform/suns_old.h"
#endif
#ifdef CONFIG_PXA250_PREMIUM
#include "platform/premium.h"
#endif
#ifdef CONFIG_SA1100_GILL
#include "platform/gill.h"
#endif
#ifdef CONFIG_SA1100_ENDA
#include "platform/enda.h"
#endif
#ifdef CONFIG_PXA250_TRIZEPS2
#include "platform/trizeps2.h"
#endif
#ifdef CONFIG_S3C2410_MPORT3
#include "platform/mport3.h"
#endif
#ifdef CONFIG_S3C2410_MPORT1
#include "platform/mport1.h"
#endif
#ifdef CONFIG_SA1100_WISMO
#include "platform/wismo.h"
#endif

#ifndef __ASSEMBLY__
extern int board_init(void);
extern int misc(void);
#endif

#endif /* _VIVI_MACHINE_H */
