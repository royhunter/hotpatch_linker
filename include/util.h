#ifndef __HOTPATCH_UTIL_H__
#define __HOTPATCH_UTIL_H__

#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdarg.h>

void debug_print(const char *fmt,...);
void *xmalloc(size_t);
void *xrealloc(void *, size_t);
char *xstrdup(const char *);
char *xstrcat(char *, const char *, size_t);

#define ERROR printf
#define DEBUG debug_print
#define INFO printf


#endif