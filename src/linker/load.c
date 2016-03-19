#include <stdio.h>


#include "util.h"
#include "file.h"

int main(int argc, char **argv)
{
	int fp;
    char *filename = argv[1];
	//struct obj_file *f;

	fp = file_open(filename, O_RDONLY);
	if (fp == -1)
	{
		error("%s open fail!\n", filename);
		return 1;
	}
    debug("%s %m open ok!\n", filename);

	//if ((f = obj_load(fp, ET_REL, filename)) == NULL)
	//	goto out;

//out:
    file_close(fp);
	return 1;

}