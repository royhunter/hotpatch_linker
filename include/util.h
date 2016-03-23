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

#define ERROR printf
#define DEBUG printf
#define INFO printf

void *xmalloc(size_t);
void *xrealloc(void *, size_t);
char *xstrdup(const char *);
char *xstrcat(char *, const char *, size_t);



#endif