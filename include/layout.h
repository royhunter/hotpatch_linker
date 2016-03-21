#ifndef __HOTPATCH_LAYOUT_H__
#define __HOTPATCH_LAYOUT_H__

#define PATCH_MAGIC 0xdeadbeef
#define PATCH_SECTION_MAX  10


typedef struct {
    char *name;
    uint64_t start;
    uint64_t size;
    char *contents;
}patch_section;


typedef struct {
    uint32_t patch_magic;
    uint32_t section_num;
    patch_section sections[PATCH_SECTION_MAX];
}patch_layout;






#endif
