#include "patch.h"
#include <arpa/inet.h>



Patch *ppatch = NULL;


void patch_init()
{
    ppatch = (Patch *)malloc(sizeof(Patch));

    memset((void *)ppatch, 0, sizeof(Patch));

    ppatch->magic = PATCH_MAGIC;

}


void patch_add_func(char *name, ElfW(Addr) addr)
{
    uint32_t index = ppatch->fp_index;
    uint32_t namelen = strlen(name);

    if(index >= PATCH_FUNC_MAX) {
        ERROR("ERROR, number of patch function should not exceed %d \n", PATCH_FUNC_MAX);
        exit(1);
    }

    if (namelen >= PATCH_FUNCNAME_LEN ){
        ERROR("ERROR Function %s's name is too long, should not exceed %d bytes\n", name, PATCH_FUNCNAME_LEN);
        exit(1);
    }

    strcpy(ppatch->pf[index].name, name);
    ppatch->pf[index].addr = addr;

    ppatch->fp_index = index + 1;

}


void patch_output(Patch *patch, char *file)
{
    int patch_fd;
    uint32_t magic;
    int count, i;
    int fp_index;
    int image_size;
    patch_fd = file_open(file, O_CREAT | O_WRONLY);
    if (patch_fd == -1)
    {
        ERROR("%s open fail!\n", file);
        return;
    }
    DEBUG("%s open ok!\n", file);

    magic = htonl(patch->magic);

    count = file_write(patch_fd, &magic, sizeof(magic));
    if (count != sizeof(magic))
        ERROR("patch write out error %d\n", __LINE__);

    fp_index = htonl(patch->fp_index);
    count = file_write(patch_fd, &fp_index, sizeof(fp_index));
    if (count != sizeof(fp_index))
        ERROR("patch write out error %d\n", __LINE__);

    for( i = 0; i < patch->fp_index; i++){
        count = file_write(patch_fd, patch->pf[i].name, PATCH_FUNCNAME_LEN);
        if (count != PATCH_FUNCNAME_LEN) {
            ERROR("patch write out error %d\n", __LINE__);
        }

        ElfW(Addr) addr = 0;
        byte_put_big_endian((unsigned char *)&addr, (elf_vma)patch->pf[i].addr, sizeof(ElfW(Addr)));
        count = file_write(patch_fd, &addr, sizeof(ElfW(Addr)));
        if (count != sizeof(ElfW(Addr))) {
            ERROR("patch write out error %d\n", __LINE__);
        }
    }

    image_size = htonl(patch->image_size);
    count = file_write(patch_fd, &image_size, sizeof(image_size));
    if (count != sizeof(image_size)) {
        ERROR("patch write out error %d\n", __LINE__);
    }


    count = file_write(patch_fd, patch->image, patch->image_size);
    if (count != patch->image_size) {
        ERROR("patch write out error %d\n", __LINE__);
    }

    file_close(patch_fd);

    INFO("PATCH OUTPUT SUCCESSFULLY!!!  =================>%s\n", file);
}



