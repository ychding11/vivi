/*
 *  vivi/lib/version.c
 */

#include "version.h"
#include "compile.h"

const char *vivi_banner =
                       "VIVI version " VIVI_RELEASE " (" VIVI_COMPILE_BY "@"
                       VIVI_COMPILE_HOST ") (" VIVI_COMPILER ") " UTS_VERSION "\r\n";
