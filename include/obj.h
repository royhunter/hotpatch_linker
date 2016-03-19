#ifndef __HOTPATCH_OBJ_H__
#define __HOTPATCH_OBJ_H__


#include <sys/types.h>
#include <elf.h>

#define ELF_MACHINE_H "elf_x86_64.h"

#include ELF_MACHINE_H


#ifndef ElfW
# if ELFCLASSM == ELFCLASS32
#  define ElfW(x)  Elf32_ ## x
#  define ELFW(x)  ELF32_ ## x
# else
#  define ElfW(x)  Elf64_ ## x
#  define ELFW(x)  ELF64_ ## x
# endif
#endif



struct obj_string_patch_struct;
struct obj_symbol_patch_struct;

struct obj_section
{
  ElfW(Shdr) header;
  const char *name;
  char *contents;
  struct obj_section *load_next;
  int idx;
};

struct obj_symbol
{
  struct obj_symbol *next;	/* hash table link */
  const char *name;
  unsigned long value;
  unsigned long size;
  int secidx;			/* the defining section index/module */
  int info;
  int ksymidx;			/* for export to the kernel symtab */
  int r_type;			/* relocation type */
};


#define HASH_BUCKETS  521


struct obj_file
{
  ElfW(Ehdr) header;
  ElfW(Addr) baseaddr;
  struct obj_section **sections;
  struct obj_section *load_order;
  struct obj_section **load_order_search_start;
  struct obj_string_patch_struct *string_patches;
  struct obj_symbol_patch_struct *symbol_patches;
  int (*symbol_cmp)(const char *, const char *);
  unsigned long (*symbol_hash)(const char *);
  unsigned long local_symtab_size;
  struct obj_symbol **local_symtab;
  struct obj_symbol *symtab[HASH_BUCKETS];
  const char *filename;
  char *persist;
};


enum obj_reloc
{
  obj_reloc_ok,
  obj_reloc_overflow,
  obj_reloc_dangerous,
  obj_reloc_unhandled,
  obj_reloc_constant_gp
};

struct obj_string_patch_struct
{
  struct obj_string_patch_struct *next;
  int reloc_secidx;
  ElfW(Addr) reloc_offset;
  ElfW(Addr) string_offset;
};

struct obj_symbol_patch_struct
{
  struct obj_symbol_patch_struct *next;
  int reloc_secidx;
  ElfW(Addr) reloc_offset;
  struct obj_symbol *sym;
};

/* Standard method of finding relocation symbols, sets isym */
#define obj_find_relsym(isym, f, find, rel, symtab, strtab) \
	{ \
		unsigned long symndx = ELFW(R_SYM)((rel)->r_info); \
		ElfW(Sym) *extsym = (symtab)+symndx; \
		if (ELFW(ST_BIND)(extsym->st_info) == STB_LOCAL) { \
			isym = (typeof(isym)) (f)->local_symtab[symndx]; \
		} \
		else { \
			const char *name; \
			if (extsym->st_name) \
				name = (strtab) + extsym->st_name; \
			else \
				name = (f)->sections[extsym->st_shndx]->name; \
			isym = (typeof(isym)) obj_find_symbol((find), name); \
		} \
	}

struct obj_file *arch_new_file (void);
struct obj_section *arch_new_section (void);
struct obj_symbol *arch_new_symbol (void);
int arch_load_proc_section(struct obj_section *sec, int fp);



unsigned long obj_elf_hash (const char *name);
void obj_insert_section_load_order (struct obj_file *f, struct obj_section *sec);
struct obj_symbol *obj_add_symbol (struct obj_file *f, const char *name, unsigned long symidx,
		int info, int secidx, ElfW(Addr) value, unsigned long size);





#endif
