#include "util.h"
#include "obj.h"

#include "elfcomm.h"

inline unsigned long
obj_elf_hash_n(const char *name, unsigned long n)
{
  unsigned long h = 0;
  unsigned long g;
  unsigned char ch;

  while (n > 0)
    {
      ch = *name++;
      h = (h << 4) + ch;
      if ((g = (h & 0xf0000000)) != 0)
	{
	  h ^= g >> 24;
	  h &= ~g;
	}
      n--;
    }
  return h;
}

unsigned long
obj_elf_hash (const char *name)
{
  return obj_elf_hash_n(name, strlen(name));
}

#if defined (ARCH_alpha)
#define ARCH_SHF_SHORT	SHF_ALPHA_GPREL
#elif defined (ARCH_ia64)
#define ARCH_SHF_SHORT	SHF_IA_64_SHORT
#else
#define ARCH_SHF_SHORT	0
#endif

struct obj_symbol *
obj_find_symbol (struct obj_file *f, const char *name)
{
  struct obj_symbol *sym;
  unsigned long hash = f->symbol_hash(name) % HASH_BUCKETS;

  for (sym = f->symtab[hash]; sym; sym = sym->next)
    if (f->symbol_cmp(sym->name, name) == 0)
      return sym;

  return NULL;
}


static int
obj_load_order_prio(struct obj_section *a)
{
    unsigned long af, ac;
    Elf32_Word sh_type = BYTE_GET(a->header.sh_type);
    af = BYTE_GET(a->header.sh_flags);

    ac = 0;
    if (a->name[0] != '.' || strlen(a->name) != 10 || strcmp(a->name + 5, ".init"))
        ac |= 64;

    if (af & SHF_ALLOC) ac |= 32;
    if (af & SHF_EXECINSTR) ac |= 16;
    if (!(af & SHF_WRITE)) ac |= 8;
    if (sh_type != SHT_NOBITS) ac |= 4;
    /* Desired order is
		    P S  AC & 7
	    .data	1 0  4
	    .got	1 1  3
	    .sdata  1 1  1
	    .sbss   0 1  1
	    .bss    0 0  0  */
    if (strcmp (a->name, ".got") == 0) ac |= 2;
    if (af & ARCH_SHF_SHORT)
        ac = (ac & ~4) | 1;

    return ac;
}

extern ElfW(Ehdr) Source_ELF_Header;
extern ElfW(Ehdr) *p_Source_ELF_Header;
extern ElfW(Shdr) *p_Source_ELF_shtab;
extern char   *p_Source_ELF_shstrtab;
extern char   *p_Source_ELF_strtab;
extern ElfW(Sym)  *p_Source_ELF_symtab;
extern uint32_t source_sym_num;

void add_symbol_from_exec(struct obj_file *f)
{
    int i;
    for( i = 0; i < source_sym_num; i++)
    {
        struct obj_symbol *sym;
        char *name = &p_Source_ELF_strtab[BYTE_GET(p_Source_ELF_symtab[i].st_name)];

        sym = obj_find_symbol(f, (char *)name);
        if( sym && ELFW(ST_BIND) (sym->info) != STB_LOCAL)
        {
            sym = obj_add_symbol(f, (char *) name, -1,
				  ELFW(ST_INFO) (STB_GLOBAL, STT_NOTYPE),
					     SHN_HIRESERVE + 2,BYTE_GET(p_Source_ELF_symtab[i].st_value), 0);

            DEBUG("%d  %s, 0x%x\n", i, name, (int)BYTE_GET(p_Source_ELF_symtab[i].st_value));
        }
    }

}

void find_symbol_from_exec(struct obj_file *f)
{
    int i;
    for( i = 0; i < source_sym_num; i++)
    {
        struct obj_symbol *sym;
        char *name = &p_Source_ELF_strtab[BYTE_GET(p_Source_ELF_symtab[i].st_name)];

        sym = obj_find_symbol(f, (char *)name);
        if( sym && ELFW(ST_BIND) (sym->info) != STB_LOCAL)
        {
            if (ELFW(ST_TYPE) (sym->info) == STT_FUNC){
                DEBUG("THIS IS A PATCH FUNC %s\n", sym->name);
                continue;
            }

            DEBUG("find symbol %s\n", sym->name);
            sym = obj_add_symbol(f, (char *) name, -1,
				  ELFW(ST_INFO) (STB_GLOBAL, STT_NOTYPE),
					     SHN_HIRESERVE + 2, BYTE_GET(p_Source_ELF_symtab[i].st_value), 0);

            DEBUG("SYM find index: %d  name: %s, 0x%x\n", i, name, (int)BYTE_GET(p_Source_ELF_symtab[i].st_value));
        }
    }
}


struct obj_symbol *
obj_add_symbol (struct obj_file *f, const char *name, unsigned long symidx, int info, int secidx, ElfW(Addr) value, unsigned long size)
{
    struct obj_symbol *sym;
    unsigned long hash = f->symbol_hash(name) % HASH_BUCKETS;
    int n_type = ELFW(ST_TYPE)(info);
    int n_binding = ELFW(ST_BIND)(info);

    for (sym = f->symtab[hash]; sym; sym = sym->next)
    {
        if (f->symbol_cmp(sym->name, name) == 0)
        {
	        int o_secidx = sym->secidx;
	        int o_info = sym->info;
	        int o_type = ELFW(ST_TYPE)(o_info);
	        int o_binding = ELFW(ST_BIND)(o_info);

            DEBUG("name:==================%s\n", name);
            DEBUG("o_type %d, o_binding %d\n", o_type, o_binding);
            DEBUG("n_type %d, n_binding %d\n", n_type, n_binding);
            /* A redefinition!  Is it legal?  */
	        if (secidx == SHN_UNDEF)
	            return sym;
	        else if (o_secidx == SHN_UNDEF)
	            goto found;
	        else if (n_binding == STB_GLOBAL && o_binding == STB_LOCAL)
	        {
    	    /* Cope with local and global symbols of the same name
    	                 in the same object file, as might have been created
    	                 by ld -r.  The only reason locals are now seen at this
    	                 level at all is so that we can do semi-sensible things
    	                 with parameters.
    	             */

	            struct obj_symbol *nsym, **p;

	            nsym = arch_new_symbol();
	            nsym->next = sym->next;
	            nsym->ksymidx = -1;

	            /* Excise the old (local) symbol from the hash chain.  */
	            for (p = &f->symtab[hash]; *p != sym; p = &(*p)->next)
	                continue;
	            *p = sym = nsym;
	            goto found;
	        }
	        else if (n_binding == STB_LOCAL)
	        {
        	    /* Another symbol of the same name has already been defined.
        	                 Just add this to the local table.
        	             */
        	    sym = arch_new_symbol();
        	    sym->next = NULL;
        	    sym->ksymidx = -1;
        	    f->local_symtab[symidx] = sym;
        	    goto found;
	        }
	        else if (n_binding == STB_WEAK)
	            return sym;
	        else if (o_binding == STB_WEAK)
	            goto found;
	            /*
	                        Don't unify COMMON symbols with object types the programmer
	                        doesn't expect.
	                     */
	        else if (secidx == SHN_COMMON
		        && (o_type == STT_NOTYPE || o_type == STT_OBJECT))
	            return sym;
	        else if (o_secidx == SHN_COMMON
		        && (n_type == STT_NOTYPE || n_type == STT_OBJECT))
	            goto found;
	        else
     	    {
         	    /* Don't report an error if the symbol is coming from
         	                 the kernel or some external module.
         	             */
         	    if (secidx <= SHN_HIRESERVE)
                    ERROR("%s multiply defined\n", name);
         	    return sym;
            }
        }
    }

    /* Completely new symbol.  */
    sym = arch_new_symbol();
    sym->next = f->symtab[hash];
    f->symtab[hash] = sym;
    sym->ksymidx = -1;

    if (ELFW(ST_BIND)(info) == STB_LOCAL && symidx != -1)
    {
        if (symidx >= f->local_symtab_size)
            ERROR("local symbol %s with index %ld exceeds local_symtab_size %ld\n", name, (long) symidx, (long) f->local_symtab_size);
        else
            DEBUG("symidx: %d\n", (int)symidx);
            f->local_symtab[symidx] = sym;
    }

found:
  sym->name = name;
  sym->value = value;
  sym->size = size;
  sym->secidx = secidx;
  sym->info = info;
  sym->r_type = 0;	/* should be R_arch_NONE for all arch */

  return sym;
}


struct obj_section *
obj_create_alloced_section (struct obj_file *f, char *name,
			    unsigned long align, unsigned long size,
			    unsigned long flags)
{
  int newidx = f->header.e_shnum++;
  struct obj_section *sec;

  f->sections = xrealloc(f->sections, (newidx+1) * sizeof(sec));
  f->sections[newidx] = sec = arch_new_section();

  memset(sec, 0, sizeof(*sec));
  sec->header.sh_type = SHT_PROGBITS;
  sec->header.sh_flags = flags | SHF_ALLOC;
  sec->header.sh_size = size;
  sec->header.sh_addralign = align;
  sec->name = name;
  sec->idx = newidx;
  if (size)
    sec->contents = xmalloc(size);

  obj_insert_section_load_order(f, sec);

  return sec;
}


void
obj_insert_section_load_order (struct obj_file *f, struct obj_section *sec)
{
    struct obj_section **p;
    int prio = obj_load_order_prio(sec);
    for (p = f->load_order_search_start; *p ; p = &(*p)->load_next)
        if (obj_load_order_prio(*p) < prio)
            break;
    sec->load_next = *p;
    *p = sec;
}


ElfW(Addr) obj_symbol_final_value (struct obj_file *f, struct obj_symbol *sym)
{
    if (sym)
    {
        //DEBUG("SECIDX: %d\n", sym->secidx);
        if (sym->secidx >= SHN_LORESERVE)
	        return sym->value;

        //DEBUG("sym->value: %d,  sym->secidx: %d, f->sections[sym->secidx]->header.sh_addr: %d\n",
            //(int)sym->value, (int)sym->secidx,  (int)f->sections[sym->secidx]->header.sh_addr);

        return sym->value + f->sections[sym->secidx]->header.sh_addr;
    }
    else
    {
        /* As a special case, a NULL sym has value zero.  */
        return 0;
    }
}

struct obj_section *
obj_find_section (struct obj_file *f, const char *name)
{
  int i, n = f->header.e_shnum;

  for (i = 0; i < n; ++i)
    if (strcmp(f->sections[i]->name, name) == 0)
      return f->sections[i];

  return NULL;
}

