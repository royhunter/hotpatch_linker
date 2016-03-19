#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>



int file_open(const char *name, int mode)
{
	return open(name, mode);
}

void file_close(int fd)
{
    close(fd);
}