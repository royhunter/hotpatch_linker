#include <string.h>
#include <assert.h>
#include <alloca.h>

#include <obj.h>
#include <util.h>




int obj_relocate (struct obj_file *f, ElfW(Addr) base)
{
    int i, n = BYTE_GET(f->header.e_shnum);
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
        if (BYTE_GET(relsec->header.sh_type) != SHT_RELM)
	        continue;

        symsec = f->sections[BYTE_GET(relsec->header.sh_link)];
        targsec = f->sections[BYTE_GET(relsec->header.sh_info)];// target for section of this rel section
        strsec = f->sections[BYTE_GET(symsec->header.sh_link)];

        if (!(BYTE_GET(targsec->header.sh_flags) & SHF_ALLOC))
	        continue;

        rel = (ElfW(RelM) *)relsec->contents;
        relend = rel + (BYTE_GET(relsec->header.sh_size) / sizeof(ElfW(RelM)));
        symtab = (ElfW(Sym) *)symsec->contents;
        nsyms = BYTE_GET(symsec->header.sh_size) / BYTE_GET(symsec->header.sh_entsize);
        strtab = (const char *)strsec->contents;

        for (; rel < relend; ++rel)
        {
            ElfW(Addr) value = 0;
            struct obj_symbol *intsym = NULL;
            unsigned long symndx;
            const char *errmsg;

            /* Attempt to find a value to use for this relocation.  */
            symndx = ELFW(R_SYM)(BYTE_GET(rel->r_info));
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
                value = obj_symbol_final_value(f, intsym);
                DEBUG("SYM VALUE: 0x%x\n", (int)value);
            }
            int n_binding = ELFW(ST_BIND)(intsym->info);
            if( value == 0 && n_binding == STB_GLOBAL)
            {
                DEBUG("no value sym: %s\n", intsym->name);
                ERROR("error: can not relocate sym: %s\n", intsym->name);
                exit(0);
            }
        #if SHT_RELM == SHT_RELA
            value += BYTE_GET(rel->r_addend);
        #endif
#if 1
        /* Do it! */
            switch (arch_apply_relocation(f,targsec,symsec,intsym,rel,value))
    	    {
    	        case obj_reloc_ok:
                    DEBUG("obj_reloc_ok\n");
    	            break;

    	        case obj_reloc_overflow:
    	            errmsg = "Relocation overflow\n";
    	            goto bad_reloc;
    	        case obj_reloc_dangerous:
    	            errmsg = "Dangerous relocation\n";
    	            goto bad_reloc;
    	        case obj_reloc_unhandled:
    	            errmsg = "Unhandled relocation\n";
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



int
obj_check_undefineds(struct obj_file *f, int quiet)
{
    unsigned long i;
    int ret = 1;

    DEBUG("called %s\n", __FUNCTION__);

    for (i = 0; i < HASH_BUCKETS; ++i)
    {
        struct obj_symbol *sym;
        for (sym = f->symtab[i]; sym ; sym = sym->next)
        if (sym->secidx == SHN_UNDEF)
        {
	        if (ELFW(ST_BIND)(sym->info) == STB_WEAK)
            {
		        sym->secidx = SHN_ABS;
		        sym->value = 0;
	        }
	        else if (sym->r_type) /* assumes R_arch_NONE is 0 on all arch */
	        {
		        if (!quiet)
			        ERROR("%s: unresolved symbol %s", f->filename, sym->name);
		        ret = 0;
	        }
	    }
    }

    return ret;
}



void
obj_allocate_commons(struct obj_file *f)
{
    struct common_entry
    {
        struct common_entry *next;
        struct obj_symbol *sym;
    } *common_head = NULL;

    unsigned long i;

    for (i = 0; i < HASH_BUCKETS; ++i)
    {
        struct obj_symbol *sym;
        for (sym = f->symtab[i]; sym ; sym = sym->next) {
    	    if (sym->secidx == SHN_COMMON)
    	    {
    	        /*
    	                    Collect all COMMON symbols and sort them by size so as to
    	                    minimize space wasted by alignment requirements.
    	                    */
    	        {
    	            struct common_entry **p, *n;
    	            for (p = &common_head; *p ; p = &(*p)->next)
    		            if (sym->size <= (*p)->sym->size)
    		                break;

        	        n = alloca(sizeof(*n));
        	        n->next = *p;
        	        n->sym = sym;
        	        *p = n;
    	        }
    	    }
        }
    }

    for (i = 1; i < f->local_symtab_size; ++i)
    {
        struct obj_symbol *sym = f->local_symtab[i];
        if (sym && sym->secidx == SHN_COMMON)
	    {
	        struct common_entry **p, *n;
	        for (p = &common_head; *p ; p = &(*p)->next)
            {
	            if (sym == (*p)->sym)
	                break;
	            else if (sym->size < (*p)->sym->size)
	            {
		            n = alloca(sizeof(*n));
		            n->next = *p;
		            n->sym = sym;
		            *p = n;
		            break;
                }
            }
        }
    }

    if (common_head)
    {
        /* Find the bss section.  */
        for (i = 0; i < BYTE_GET(f->header.e_shnum); ++i)
            if (BYTE_GET(f->sections[i]->header.sh_type) == SHT_NOBITS)
                break;

        /* If for some reason there hadn't been one, create one.  */
        if (i == BYTE_GET(f->header.e_shnum))
        {
            struct obj_section *sec;

            f->sections = xrealloc(f->sections, (i+1) * sizeof(sec));
            f->sections[i] = sec = arch_new_section();

            BYTE_PUT(f->header.e_shnum, i+1);

            memset(sec, 0, sizeof(*sec));
            BYTE_PUT(sec->header.sh_type, SHT_PROGBITS);
            BYTE_PUT(sec->header.sh_flags, SHF_WRITE|SHF_ALLOC);
            sec->name = ".bss";
            sec->idx = i;
        }

        /* Allocate the COMMONS.  */
        {
            ElfW(Addr) bss_size = BYTE_GET(f->sections[i]->header.sh_size);
            ElfW(Addr) max_align = BYTE_GET(f->sections[i]->header.sh_addralign);
            struct common_entry *c;

            for (c = common_head; c ; c = c->next)
            {
                ElfW(Addr) align = c->sym->value;

                if (align > max_align)
	                max_align = align;
                if (bss_size & (align - 1))
                    bss_size = (bss_size | (align - 1)) + 1;

                c->sym->secidx = i;
                c->sym->value = bss_size;

	            bss_size += c->sym->size;
            }

            BYTE_PUT(f->sections[i]->header.sh_size, bss_size);
            BYTE_PUT(f->sections[i]->header.sh_addralign, max_align);
        }
    }

    /* For the sake of patch relocation and parameter initialization,
            allocate zeroed data for NOBITS sections now.  Note that after
            this we cannot assume NOBITS are really empty.  */
    for (i = 0; i < BYTE_GET(f->header.e_shnum); ++i)
    {
        struct obj_section *s = f->sections[i];
        if (BYTE_GET(s->header.sh_type) == SHT_NOBITS)
        {
            if (BYTE_GET(s->header.sh_size))
                s->contents = memset(xmalloc(s->header.sh_size), 0, s->header.sh_size);
            else
                s->contents = NULL;

            BYTE_PUT(s->header.sh_type, SHT_PROGBITS);
        }
    }
}

unsigned long
obj_load_size (struct obj_file *f)
{
    unsigned long dot = 0;
    struct obj_section *sec;

    /* Finalize the positions of the sections relative to one another.  */

    for (sec = f->load_order; sec ; sec = sec->load_next)
    {
        ElfW(Addr) align;

        align = BYTE_GET(sec->header.sh_addralign);
        if (align && (dot & (align - 1)))
            dot = (dot | (align - 1)) + 1;

        BYTE_PUT(sec->header.sh_addr, dot);
        dot += BYTE_GET(sec->header.sh_size);
    }

    return dot;
}


int
obj_create_image (struct obj_file *f, char *image)
{
    struct obj_section *sec;
    ElfW(Addr) base = f->baseaddr;

  for (sec = f->load_order; sec ; sec = sec->load_next)
    {
      char *secimg;

      if (sec->contents == 0)
	continue;

      secimg = image + (sec->header.sh_addr - base);

      /* Note that we allocated data for NOBITS sections earlier.  */
      memcpy(secimg, sec->contents, sec->header.sh_size);
    }

  return 1;
}
