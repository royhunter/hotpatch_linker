#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "util.h"

#include "obj.h"
#include "elfcomm.h"


struct mips_hi16
{
  struct mips_hi16 *next;
  Elf32_Addr *addr;
  Elf32_Addr value;
};

struct mips_file
{
  struct obj_file root;
  struct mips_hi16 *mips_hi16_list;
};


struct obj_file *
arch_new_file (void)
{
    struct mips_file *mf;

    mf = xmalloc(sizeof(*mf));
    mf->mips_hi16_list = NULL;

    return (struct obj_file *) mf;
}

struct obj_section *
arch_new_section (void)
{
    return xmalloc(sizeof(struct obj_section));
}

struct obj_symbol *
arch_new_symbol (void)
{
    return xmalloc(sizeof(struct obj_symbol));
}


int
arch_create_got (struct obj_file *f)
{
    return 1;
}

int
arch_load_proc_section(struct obj_section *sec, int fp)
{
    Elf32_Word sh_type = BYTE_GET(sec->header.sh_type);
    switch (sh_type)
    {
        case SHT_MIPS_DEBUG:
        case SHT_MIPS_REGINFO:
        case SHT_MIPS_DWARF:
            /* Ignore debugging sections  */
            sec->contents = NULL;
            break;

        case SHT_MIPS_LIBLIST:
        case SHT_MIPS_CONFLICT:
        case SHT_MIPS_GPTAB:
        case SHT_MIPS_UCODE:
        case SHT_MIPS_OPTIONS:
        case SHT_MIPS_EVENTS:
            /* These shouldn't ever be in a module file.  */
            ERROR("Unhandled section header type: %08x", sh_type);
            return -1;

        default:
            /* We don't even know the type.  This time it might as well be a
	            supernova.  */
            ERROR("Unknown section header type: %08x", sh_type);
            return -1;
    }

    return 0;
}

enum obj_reloc
arch_apply_relocation (struct obj_file *f,
		       struct obj_section *targsec,
		       struct obj_section *symsec,
		       struct obj_symbol *sym,
		       Elf32_Rel *rel,
		       Elf32_Addr v)
{
    Elf32_Addr ins;
    struct mips_file *mf = (struct mips_file *)f;
    Elf32_Addr *loc = (Elf32_Addr *)(targsec->contents + BYTE_GET(rel->r_offset));
    Elf32_Addr dot = BYTE_GET(targsec->header.sh_addr) + BYTE_GET(rel->r_offset);
    enum obj_reloc ret = obj_reloc_ok;
    DEBUG("loc: 0x%x, dot: 0x%x\n", (int)BYTE_GET_BY_ADDR(loc, sizeof(Elf32_Addr)), (int)dot);
    /* _gp_disp is a magic symbol for PIC which is not supported for
         the kernel and loadable modules.  */
    if (strcmp(sym->name, "_gp_disp") == 0)
        ret = obj_reloc_unhandled;

    DEBUG("ELF32_R_TYPE %d\n", (int)ELF32_R_TYPE(BYTE_GET(rel->r_info)));
    switch (ELF32_R_TYPE(BYTE_GET(rel->r_info)))
    {
        case R_MIPS_NONE:
            break;

        case R_MIPS_32:
            ins = BYTE_GET_BY_ADDR(loc, sizeof(Elf32_Addr)) + v;
            BYTE_PUT_BY_ADDR(loc, ins, sizeof(Elf32_Addr));
            break;

        case R_MIPS_26:
            if (v % 4)
    	        ret = obj_reloc_dangerous;
            if ((v & 0xf0000000) != ((dot + 4) & 0xf0000000))
            	ret = obj_reloc_overflow;
            BYTE_PUT_BY_ADDR(loc, (BYTE_GET_BY_ADDR(loc, sizeof(Elf32_Addr)) & ~0x03ffffff) | ((BYTE_GET_BY_ADDR(loc, sizeof(Elf32_Addr)) + (v >> 2)) & 0x03ffffff), sizeof(Elf32_Addr));
            break;

        case R_MIPS_HI16:
        {
	        struct mips_hi16 *n;
            /* We cannot relocate this one now because we don't know the value
        	        of the carry we need to add.  Save the information, and let LO16
        	        do the actual relocation.  */
        	n = (struct mips_hi16 *) xmalloc (sizeof *n);
        	n->addr = loc;
        	n->value = v;
        	n->next = mf->mips_hi16_list;
        	mf->mips_hi16_list = n;
        	break;
        }

        case R_MIPS_LO16:
        {
        	Elf32_Addr val, vallo;
            unsigned long insnlo = BYTE_GET_BY_ADDR(loc, sizeof(Elf32_Addr));
        	/* Sign extend the addend we extract from the lo insn.  */
        	vallo = ((insnlo & 0xffff) ^ 0x8000) - 0x8000;

	        if (mf->mips_hi16_list != NULL)
	        {
        	    struct mips_hi16 *l;

        	    l = mf->mips_hi16_list;
        	    while (l != NULL)
	            {
            		struct mips_hi16 *next;
            		unsigned long insn;

            		/* The value for the HI16 had best be the same. */
            		assert(v == l->value);

            		/* Do the HI16 relocation.  Note that we actually don't
            		                need to know anything about the LO16 itself, except where
            		                to find the low 16 bits of the addend needed by the LO16.  */
                    insn = BYTE_GET_BY_ADDR(l->addr, sizeof(Elf32_Addr));
            		val = ((insn & 0xffff) << 16) + vallo;
            		val += v;

            		/* Account for the sign extension that will happen in the
            		                low bits.  */
            		val = ((val >> 16) + ((val & 0x8000) != 0)) & 0xffff;

            		insn = (insn &~ 0xffff) | val;
                    BYTE_PUT_BY_ADDR(l->addr, insn, sizeof(Elf32_Addr));
                    DEBUG("after relocate, loc: 0x%x\n", (int)insn);
            		next = l->next;
            		free(l);
            		l = next;
	            }

	            mf->mips_hi16_list = NULL;
	        }

    	    /* Ok, we're done with the HI16 relocs.  Now deal with the LO16.  */
    	    val = v + vallo;
        	insnlo = (insnlo & ~0xffff) | (val & 0xffff);

            BYTE_PUT_BY_ADDR(loc, insnlo, sizeof(Elf32_Addr));
            DEBUG("after relocate, loc: 0x%x\n", (int)insnlo);
        	break;
        }

        default:
            ret = obj_reloc_unhandled;
            break;
    }

    return ret;
}

int
arch_finalize_section_address(struct obj_file *f, Elf32_Addr base)
{
    int  i, n = BYTE_GET(f->header.e_shnum);
    DEBUG("Relocate base address is 0x%x\n", (int)base);
    f->baseaddr = base;
    for (i = 0; i < n; ++i)
        BYTE_PUT(f->sections[i]->header.sh_addr, BYTE_GET(f->sections[i]->header.sh_addr)+base);

    return 1;
}

