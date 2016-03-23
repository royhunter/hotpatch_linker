#ifndef __HOTPATCH_ELFCOMM_H__
#define __HOTPATCH_ELFCOMM_H__

#include "util.h"
#ifdef PLATFORM_X86_64
/* We can't use any bfd types here since readelf may define BFD64 and
   objdump may not.  */
#define HOST_WIDEST_INT	long long
#endif
#ifdef PLATFORM_MIPS32
#define HOST_WIDEST_INT long
#endif

typedef unsigned HOST_WIDEST_INT elf_vma;

extern void (*byte_put) (unsigned char *, elf_vma, int);
extern void byte_put_little_endian (unsigned char *, elf_vma, int);
extern void byte_put_big_endian (unsigned char *, elf_vma, int);

extern elf_vma (*byte_get) (unsigned char *, int);
extern elf_vma byte_get_signed (unsigned char *, int);
extern elf_vma byte_get_little_endian (unsigned char *, int);
extern elf_vma byte_get_big_endian (unsigned char *, int);

#define BYTE_PUT(field, val)	byte_put (field, val, sizeof (field))
//#define BYTE_GET(field)		byte_get (field, sizeof (field))
//#define BYTE_GET(field, size)		byte_get (field, size)
#define BYTE_GET(field)  byte_get((void *)&field, sizeof(field))
#define BYTE_GET_SIGNED(field)	byte_get_signed (field, sizeof (field))

#endif
