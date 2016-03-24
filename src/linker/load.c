#include <stdio.h>


#include "util.h"
#include "file.h"
#include "obj.h"



void usage()
{
    INFO("usage:...\n");
    INFO("linker elf_file obj_file\n");
}

static void print_load_map(struct obj_file *f)
{
	struct obj_symbol *sym;
	struct obj_symbol **all, **p;
	struct obj_section *sec;
	int load_map_cmp(const void *a, const void *b) {
		struct obj_symbol **as = (struct obj_symbol **) a;
		struct obj_symbol **bs = (struct obj_symbol **) b;
		unsigned long aa = obj_symbol_final_value(f, *as);
		unsigned long ba = obj_symbol_final_value(f, *bs);
		 return aa < ba ? -1 : aa > ba ? 1 : 0;
	}
	int i, nsyms, *loaded;
    INFO("========================================\n");
	/* Report on the section layout.  */

	INFO("Sections:       Size      %-*s  Align\n",
		(int) (2 * sizeof(void *)), "Address");

	for (sec = f->load_order; sec; sec = sec->load_next) {
		int a;
		unsigned long tmp;

		for (a = -1, tmp = BYTE_GET(sec->header.sh_addralign); tmp; ++a)
			tmp >>= 1;
		if (a == -1)
			a = 0;

		INFO("%-15s %08lx  %0*lx  2**%d\n",
			sec->name,
			(long)BYTE_GET(sec->header.sh_size),
			(int) (2 * sizeof(void *)),
			(long)BYTE_GET(sec->header.sh_addr),
			a);
	}

	/* Quick reference which section indicies are loaded.  */

	loaded = alloca(sizeof(int) * (i = BYTE_GET(f->header.e_shnum)));
	while (--i >= 0)
		loaded[i] = (BYTE_GET(f->sections[i]->header.sh_flags) & SHF_ALLOC) != 0;

	/* Collect the symbols we'll be listing.  */

	for (nsyms = i = 0; i < HASH_BUCKETS; ++i)
		for (sym = f->symtab[i]; sym; sym = sym->next)
			if (sym->secidx <= SHN_HIRESERVE
			    && (sym->secidx >= SHN_LORESERVE || loaded[sym->secidx]))
				++nsyms;

	all = alloca(nsyms * sizeof(struct obj_symbol *));

	for (i = 0, p = all; i < HASH_BUCKETS; ++i)
		for (sym = f->symtab[i]; sym; sym = sym->next)
			if (sym->secidx <= SHN_HIRESERVE
			    && (sym->secidx >= SHN_LORESERVE || loaded[sym->secidx]))
				*p++ = sym;

	/* Sort them by final value.  */
	qsort(all, nsyms, sizeof(struct obj_file *), load_map_cmp);

	/* And list them.  */
	INFO("\nSymbols:\n");
	for (p = all; p < all + nsyms; ++p) {
		char type = '?';
		unsigned long value;

		sym = *p;
		if (sym->secidx == SHN_ABS) {
			type = 'A';
			value = sym->value;
		} else if (sym->secidx == SHN_UNDEF) {
			type = 'U';
			value = 0;
		} else {
			struct obj_section *sec = f->sections[sym->secidx];

			if (BYTE_GET(sec->header.sh_type) == SHT_NOBITS)
				type = 'B';
			else if (BYTE_GET(sec->header.sh_flags) & SHF_ALLOC) {
				if (BYTE_GET(sec->header.sh_flags) & SHF_EXECINSTR)
					type = 'T';
				else if (BYTE_GET(sec->header.sh_flags) & SHF_WRITE)
					type = 'D';
				else
					type = 'R';
			}
			value = sym->value + BYTE_GET(sec->header.sh_addr);
		}

		if (ELFW(ST_BIND) (sym->info) == STB_LOCAL)
			type = tolower(type);

		INFO("%0*lx %c %s\n", (int) (2 * sizeof(void *)), value,
			type, sym->name);
	}
}

int main(int argc, char **argv)
{
    int ret;
	int elf_fd;
    int obj_fd;
    unsigned long image_size;
    struct obj_file *obj_f;
    void *image;

    if ( argc < 3 )
    {
        ERROR("argument err!\n");
        usage();
        return 1;
    }

    char *elf_filename = argv[1];
    char *obj_filename = argv[2];

	elf_fd = file_open(elf_filename, O_RDONLY);
	if (elf_fd == -1)
	{
		ERROR("%s open fail!\n", elf_filename);
		return 1;
	}
    DEBUG("%s open ok!\n", elf_filename);

    obj_fd = file_open(obj_filename, O_RDONLY);
	if (obj_fd == -1)
	{
		ERROR("%s open fail!\n", obj_filename);
        file_close(elf_fd);
        return 1;
	}
    DEBUG("%s open ok!\n", obj_filename);

	if ((obj_f = obj_load(obj_fd, ET_REL, obj_filename)) == NULL)
		goto out;

    INFO("obj_load ok!\n");

    INFO("load_elf_symbol....!\n");
    ret = load_exec_symbol(elf_fd);
    if (ret == -1)
        goto out;

    INFO("load_elf_symbol ok!\n");

    //add_symbol_from_exec(obj_f);
    find_symbol_from_exec(obj_f);

    arch_create_got(obj_f);

    obj_check_undefineds(obj_f, 1);

    obj_allocate_commons(obj_f);

    image_size = obj_load_size(obj_f);
    INFO("image size 0x%x\n", (int)image_size);

    obj_relocate(obj_f, 0);

    /*
	 * Whew!  All of the initialization is complete.
	 * Collect the final  image.
	 */
    image = xmalloc(image_size);
	obj_create_image(obj_f, image);

    print_load_map(obj_f);

out:
    file_close(elf_fd);
    file_close(obj_fd);
	return 1;

}