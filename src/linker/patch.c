#include "patch.h"



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






