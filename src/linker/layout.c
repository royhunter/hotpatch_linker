#include <stdio.h>


#include "util.h"
#include "file.h"
#include "obj.h"
#include "layout.h"



patch_layout pat_lay;


void format_patch_layout(struct obj_file *f)
{
    int i;
    struct obj_section *sec;
    patch_section *ps, *pps;
    memset(&pat_lay, 0, sizeof(patch_layout));

    pat_lay.patch_magic = PATCH_MAGIC;

    i = 0;
    sec = obj_find_section(f, ".text"); // 1  ".text"
    ps = &pat_lay.sections[i];
    ps->name = sec->name;
    ps->start = 0;
    ps->size = sec->header.sh_size;
    i++;

    sec = obj_find_section(f, ".data"); // 2  ".data"
    if (sec) {
        pps = &pat_lay.sections[i-1];
        ps = &pat_lay.sections[i];
        ps->name = sec->name;
        ps->start = pps->start + pps->size;
        ps->size = sec->header.sh_size;
        i++;
    }

    sec = obj_find_section(f, ".bss"); // 3  ".bss"
    if (sec) {
        pps = &pat_lay.sections[i-1];
        ps = &pat_lay.sections[i];
        ps->name = sec->name;
        ps->start = pps->start + pps->size;
        ps->size = sec->header.sh_size;
        i++;
    }

    sec = obj_find_section(f, ".rodata"); // 4  ".rodata"
    if (sec) {
        pps = &pat_lay.sections[i-1];
        ps = &pat_lay.sections[i];
        ps->name = sec->name;
        ps->start = pps->start + pps->size;
        ps->size = sec->header.sh_size;
    }






}







