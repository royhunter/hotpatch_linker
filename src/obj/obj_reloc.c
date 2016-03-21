#include <string.h>
#include <assert.h>
#include <alloca.h>

#include <obj.h>
#include <util.h>




int obj_relocate (struct obj_file *f, ElfW(Addr) base)
{
    int i, n = f->header.e_shnum;
    int ret = 1;

    /* Finalize the addresses of the sections.  */
    arch_finalize_section_address(f, base);


    /* And iterate over all of the relocations.  */
    for (i = 0; i < n; ++i)
    {
        struct obj_section *relsec, *symsec, *targsec, *strsec;
        ElfW(RelM) *rel, *relend;
        ElfW(Sym) *symtab;
        const char *strtab;
        unsigned long nsyms;

        relsec = f->sections[i];
        if (relsec->header.sh_type != SHT_RELM)
	        continue;
        DEBUG("RELSEC sh_info %d\n", (int)relsec->header.sh_info);
        symsec = f->sections[relsec->header.sh_link];
        targsec = f->sections[relsec->header.sh_info];// target for section of this rel section
        strsec = f->sections[symsec->header.sh_link];

        if (!(targsec->header.sh_flags & SHF_ALLOC))
	        continue;

        rel = (ElfW(RelM) *)relsec->contents;
        relend = rel + (relsec->header.sh_size / sizeof(ElfW(RelM)));
        symtab = (ElfW(Sym) *)symsec->contents;
        nsyms = symsec->header.sh_size / symsec->header.sh_entsize;
        strtab = (const char *)strsec->contents;

        for (; rel < relend; ++rel)
        {
            ElfW(Addr) value = 0;
            struct obj_symbol *intsym = NULL;
            unsigned long symndx;
            const char *errmsg;

            /* Attempt to find a value to use for this relocation.  */
            symndx = ELFW(R_SYM)(rel->r_info);
            if (symndx)
            {
                /* Note we've already checked for undefined symbols.  */
                if (symndx >= nsyms)
                {
                    ERROR("%s: Bad symbol index: %08lx >= %08lx",
			            f->filename, symndx, nsyms);
		            continue;
                }
                obj_find_relsym(intsym, f, f, rel, symtab, strtab);
                DEBUG("WANT TO FIND SYM: %s\n", intsym->name);
                value = obj_symbol_final_value(intsym);
                DEBUG("SYM VALUE: 0x%x\n", (int)value);
            }
        #if SHT_RELM == SHT_RELA
            value += rel->r_addend;
        #endif
#if 1
        /* Do it! */
            switch (arch_apply_relocation(f,targsec,symsec,intsym,rel,value))
    	    {
    	        case obj_reloc_ok:
    	            break;

    	        case obj_reloc_overflow:
    	            errmsg = "Relocation overflow";
    	            goto bad_reloc;
    	        case obj_reloc_dangerous:
    	            errmsg = "Dangerous relocation";
    	            goto bad_reloc;
    	        case obj_reloc_unhandled:
    	            errmsg = "Unhandled relocation";
    	            goto bad_reloc;
    	        case obj_reloc_constant_gp:
    	            errmsg = "Modules compiled with -mconstant-gp cannot be loaded";
    	            goto bad_reloc;
    bad_reloc:
    	        ERROR("%s: %s of type %ld for %s", f->filename, errmsg,
    		        (long)ELFW(R_TYPE)(rel->r_info), intsym->name);
    	        ret = 0;
    	        break;
    	    }
       #endif
        }
    }
    return ret;
}


