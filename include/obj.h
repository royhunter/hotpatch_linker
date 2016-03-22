#ifndef __HOTPATCH_OBJ_H__
#define __HOTPATCH_OBJ_H__
#include <sys/types.h>
#include <elf.h>

#define ELF_MACHINE_H "elf_x86_64.h"


//#define __MIPSEB__
//#define ELF_MACHINE_H "elf_mips.h"


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
  char *name;
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


struct obj_symbol *
obj_find_symbol (struct obj_file *f, const char *name);



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
int arch_finalize_section_address (struct obj_file *f, ElfW(Addr) base);
int arch_create_got (struct obj_file *f);


enum obj_reloc arch_apply_relocation (struct obj_file *f,
				      struct obj_section *targsec,
				      struct obj_section *symsec,
				      struct obj_symbol *sym,
				      ElfW(RelM) *rel, ElfW(Addr) value);

struct obj_file * obj_load (int fp, Elf32_Half e_type, const char *filename);
unsigned long obj_elf_hash (const char *name);
void obj_insert_section_load_order (struct obj_file *f, struct obj_section *sec);
struct obj_symbol *obj_add_symbol (struct obj_file *f, const char *name, unsigned long symidx,
		int info, int secidx, ElfW(Addr) value, unsigned long size);

int load_elf_symbol(int fd);
ElfW(Addr) obj_symbol_final_value (struct obj_file *f, struct obj_symbol *sym);
int obj_relocate (struct obj_file *f, ElfW(Addr) base);
struct obj_section *obj_find_section (struct obj_file *f, const char *name);
int obj_check_undefineds(struct obj_file *f, int quiet);
void obj_allocate_commons(struct obj_file *f);
unsigned long obj_load_size (struct obj_file *f);
int obj_create_image (struct obj_file *f, char *image);

struct obj_section *obj_create_alloced_section (struct obj_file *f, char *name,
			    unsigned long align, unsigned long size,
			    unsigned long flags);


void add_symbol_from_exec(struct obj_file *f);


#endif
