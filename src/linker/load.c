#include <stdio.h>


#include "util.h"
#include "file.h"
#include "obj.h"

void usage()
{
    INFO("usage:...\n");
}

int main(int argc, char **argv)
{
	int fp;
    char *filename = argv[1];
	struct obj_file *f;

    if ( argc < 2 )
    {
        ERROR("argument err!\n");
        usage();
        return 1;
    }

	fp = file_open(filename, O_RDONLY);
	if (fp == -1)
	{
		ERROR("%s open fail!\n", filename);
		return 1;
	}
    DEBUG("%s open ok!\n", filename);

	if ((f = obj_load(fp, ET_REL, filename)) == NULL)
		goto out;

out:
    file_close(fp);
	return 1;

}