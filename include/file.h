#ifndef __HOTPATCH_FILE_H__
#define __HOTPATCH_FILE_H__

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


extern int file_open(const char *name, int mode);
extern void file_close(int fd);



#endif