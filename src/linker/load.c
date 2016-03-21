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
    int ret;
	int elf_fd;
    int obj_fd;
    struct obj_file *obj_f;

    if ( argc < 3 )
    {
        ERROR("argument err!\n");
        usage();
        return 1;
    }

    char *elf_filename = argv[1];
    char *obj_filename = argv[2];

	elf_fd = file_open(elf_filename, O_RDONLY);
	if (elf_fd == -1)
	{
		ERROR("%s open fail!\n", elf_filename);
		return 1;
	}
    DEBUG("%s open ok!\n", elf_filename);

    obj_fd = file_open(obj_filename, O_RDONLY);
	if (obj_fd == -1)
	{
		ERROR("%s open fail!\n", obj_filename);
        file_close(elf_fd);
        return 1;
	}
    DEBUG("%s open ok!\n", obj_filename);

	if ((obj_f = obj_load(obj_fd, ET_REL, obj_filename)) == NULL)
		goto out;

    INFO("obj_load ok!\n");

    //format_patch_layout(obj_f);

    INFO("load_elf_symbol....!\n");
    ret = load_elf_symbol(elf_fd);
    if (ret == -1)
        goto out;

    INFO("load_elf_symbol ok!\n");


    obj_relocate(obj_f, 0);

    //sleep(10000);

out:
    file_close(elf_fd);
    file_close(obj_fd);
	return 1;

}