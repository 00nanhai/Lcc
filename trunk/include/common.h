#ifndef _CONMON_H_
#define _CONMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "my_log.h"

//#ifdef _SW_DEBUG_
//#define Assert(exp) assert(exp)
//#define Perror(str) perror(str)
//#else
#define Assert(exp) (void)((exp) || (_assert(#exp), 0))
static inline void _assert(const void *exp)
{
	LOG_ERRO("Assertion failed: %s", (const char *)exp);
	//exit(0);
	//abort();
}

#define Perror(x) do { \
	LOG_ERRO("%s:%s", (x), strerror(errno)); \
} while (0)
//#endif

#endif
