#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "util.h"

#include "obj.h"



struct x86_64_got_entry
{
  long int offset;
  unsigned offset_done : 1;
  unsigned reloc_done : 1;
};


struct x86_64_file
{
  struct obj_file root;
  struct obj_section *got;
};


struct x86_64_symbol
{
  struct obj_symbol root;
  struct x86_64_got_entry gotent;
};



struct obj_file *
arch_new_file (void)
{
  struct x86_64_file *f;
  f = xmalloc(sizeof(*f));
  f->got = NULL;
  return &f->root;
}
struct obj_symbol *
arch_new_symbol (void)
{
  struct x86_64_symbol *sym;
  sym = xmalloc(sizeof(*sym));
  memset(&sym->gotent, 0, sizeof(sym->gotent));
  return &sym->root;
}

struct obj_section *
arch_new_section (void)
{
  return xmalloc(sizeof(struct obj_section));
}

int
arch_load_proc_section(struct obj_section *sec, int fp)
{
    /* Assume it's just a debugging section that we can safely
       ignore ...  */
    sec->contents = NULL;

    return 0;
}



int
arch_finalize_section_address(struct obj_file *f, Elf64_Addr base)
{
    int  i, n = f->header.e_shnum;

    f->baseaddr = base;
    for (i = 0; i < n; ++i) {
        DEBUG("section: %d,  addr is 0x%x\n", i, (int)f->sections[i]->header.sh_addr);
        f->sections[i]->header.sh_addr += base;
    }

    return 1;
}




int
arch_create_got (struct obj_file *f)
{
    struct x86_64_file *ifile = (struct x86_64_file *)f;
    int i, n, offset = 0, gotneeded = 0;

    n = ifile->root.header.e_shnum;
    for (i = 0; i < n; ++i)
    {
        struct obj_section *relsec, *symsec, *strsec;
        Elf64_Rela *rel, *relend;
        Elf64_Sym *symtab;
        const char *strtab;

        relsec = ifile->root.sections[i];
        if (relsec->header.sh_type != SHT_REL)
            continue;

        symsec = ifile->root.sections[relsec->header.sh_link];
        strsec = ifile->root.sections[symsec->header.sh_link];

        rel = (Elf64_Rela *)relsec->contents;
        relend = rel + (relsec->header.sh_size / sizeof(Elf64_Rela));
        symtab = (Elf64_Sym *)symsec->contents;
        strtab = (const char *)strsec->contents;

        for (; rel < relend; ++rel)
	    {
	        struct x86_64_symbol *intsym;

	        switch (ELF64_R_TYPE(rel->r_info))
	        {
	            case R_X86_64_GOTPCREL:
	            case R_X86_64_GOT32:
	                gotneeded = 1;
	            default:
	                continue;
	        }

	        obj_find_relsym(intsym, f, &ifile->root, rel, symtab, strtab);

	        if (!intsym->gotent.offset_done)
	        {
	            intsym->gotent.offset_done = 1;
	            intsym->gotent.offset = offset;
	            offset += 4;
	        }
	    }
    }

    if (offset > 0 || gotneeded)
        ifile->got = obj_create_alloced_section(&ifile->root, ".got", 8, offset, SHF_WRITE);

  return 1;
}



enum obj_reloc
arch_apply_relocation (struct obj_file *f,
		       struct obj_section *targsec,
		       struct obj_section *symsec,
		       struct obj_symbol *sym,
		       Elf64_Rela *rel,
		       Elf64_Addr v)
{
    struct x86_64_file *ifile = (struct x86_64_file *)f;
    struct x86_64_symbol *isym  = (struct x86_64_symbol *)sym;

    Elf64_Addr *loc = (Elf64_Addr *)(targsec->contents + rel->r_offset);
    //DEBUG("TARGSEC CONTENT %p\n", targsec->contents);
    Elf64_Addr dot = targsec->header.sh_addr + rel->r_offset;
    //Elf64_Addr got = ifile->got ? ifile->got->header.sh_addr : 0;

    enum obj_reloc ret = obj_reloc_ok;

    switch (ELF64_R_TYPE(rel->r_info))
    {
        case R_X86_64_NONE:
            break;

        case R_X86_64_64:
            *loc += v;
            break;

        case R_X86_64_32:
            *(unsigned int *) loc += v;
            if (v > 0xffffffff)
            {
	            ret = obj_reloc_overflow; /* Kernel module compiled without -mcmodel=kernel. */
	            ERROR("Possibly is module compiled without -mcmodel=kernel!");
            }
            break;

        case R_X86_64_32S:
            *(signed int *) loc += v;
            break;

        case R_X86_64_16:
            *(unsigned short *) loc += v;
            break;

        case R_X86_64_8:
            *(unsigned char *) loc += v;
            break;

        case R_X86_64_PC32:
            *(unsigned int *) loc += v - dot;
            break;

        case R_X86_64_PC16:
            *(unsigned short *) loc += v - dot;
            break;

        case R_X86_64_PC8:
            *(unsigned char *) loc += v - dot;
            break;

        case R_X86_64_GLOB_DAT:
        case R_X86_64_JUMP_SLOT:
            *loc = v;
            break;

        case R_X86_64_RELATIVE:
            *loc += f->baseaddr;
            break;

        case R_X86_64_GOT32:
        case R_X86_64_GOTPCREL:
            assert(isym != NULL);
            if (!isym->gotent.reloc_done)
	        {
	            isym->gotent.reloc_done = 1;
	            *(Elf64_Addr *)(ifile->got->contents + isym->gotent.offset) = v;
	        }
            /* XXX are these really correct?  */
            if (ELF64_R_TYPE(rel->r_info) == R_X86_64_GOTPCREL)
                *(unsigned int *) loc += v + isym->gotent.offset;
            else
                *loc += isym->gotent.offset;
            break;

        default:
            ret = obj_reloc_unhandled;
            break;
    }

    return ret;
}


