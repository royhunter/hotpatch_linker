#ifndef __HOTPATCH_PATCH_H__
#define __HOTPATCH_PATCH_H__

#include "obj.h"
#include "util.h"

#define PATCH_MAGIC 0xdeadbeef

#define PATCH_FUNC_MAX 100
#define PATCH_FUNCNAME_LEN 64

typedef struct {
    char name[PATCH_FUNCNAME_LEN];
    ElfW(Addr) addr;
}PatchFunc;


typedef struct {
    uint32_t magic;
    uint32_t fp_index;
    PatchFunc pf[PATCH_FUNC_MAX];
    char *image;
}Patch;



extern void patch_init();
extern void patch_add_func(char *name, ElfW(Addr) addr);







#endif
