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

off_t file_lseek(int fd, off_t offset, int whence)
{
    return lseek(fd, offset, whence);
}

int file_read(int fd, void *buf, size_t count)
{
    return read(fd, buf, count);
}

int file_seek_read(int fd, off_t offset, int whence, void *buf, size_t count)
{
    lseek(fd, offset, whence);
    return read(fd, buf, count);
}

int file_write(int fd, void *buf, size_t count)
{
    return write(fd, buf, count);
}

