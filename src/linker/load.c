#include <stdio.h>

#include <getopt.h>
#include "util.h"
#include "file.h"
#include "obj.h"
#include "version.h"
#include "patch.h"

void usage()
{
    fputs("Usage:\n"
          "        Arch_linker [options] -o obj_filename  -e executable_filename \n"
          "        Arch: x86_64,mips32\n"
          "Options:\n"
	      "        -d  --debug             Output debug info\n"
	      "        -v, --version           Show version\n"
	      "        -h, --help              Show this help\n"
	      "        -o obj,  --obj=NAME     Set obj file name\n"
	      "        -e exec, --exec=NAME    Set exec file name\n"
          ,stderr);
    exit(1);
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
    INFO("Image info:\n");
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

extern int debug;

int main(int argc, char **argv)
{
    int ret;
    int ch;
	int elf_fd;
    int obj_fd;
    unsigned long image_size;
    struct obj_file *obj_f;
    void *image;
    char *obj_filename = NULL, *exec_filename = NULL;

    struct option long_opts[] = {
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'v'},
        {"debug", 0, 0, 'd'},
        {"obj", 0, 0, 'o'},
        {"exec", 0, 0, 'e'},
		{0, 0, 0, 0}
	};

    while((ch = getopt_long(argc, argv, "dhvo:e:", &long_opts[0], NULL)) != EOF) {
        switch(ch){
            case 'd':
                debug = 1;
                break;
            case 'o':
                obj_filename = optarg;
                break;
            case 'e':
                exec_filename = optarg;
                break;
            case 'h':
                usage();
                break;
            case 'v':
                fputs("Linker version " LINKER_VERSION "\n", stderr);
                exit(0);
                break;
            default:
                printf("Unknown param!\n");
                usage();
                exit(0);
        }

    }

    if( NULL == obj_filename || NULL == exec_filename) {
        usage();
        exit(0);
    }

    patch_init();

	elf_fd = file_open(exec_filename, O_RDONLY);
	if (elf_fd == -1)
	{
		ERROR("%s open fail!\n", exec_filename);
		return 1;
	}
    DEBUG("%s open ok!\n", exec_filename);

    obj_fd = file_open(obj_filename, O_RDONLY);
	if (obj_fd == -1)
	{
		ERROR("%s open fail!\n", obj_filename);
        file_close(elf_fd);
        return 1;
	}
    DEBUG("%s open ok!\n", obj_filename);

    INFO("Obj file load....\n");
	if ((obj_f = obj_load(obj_fd, ET_REL, obj_filename)) == NULL){
        ERROR("Obj file load failed!\n");
        goto out;
    }

    INFO("Obj file load ok!\n");
    DEBUG("=================================================================\n");

    INFO("Exec symbol load....\n");
    ret = load_exec_symbol(elf_fd);
    if (ret == -1) {
        ERROR("Exec symbol load failed!\n");
        goto out;
    }

    INFO("Load exec symbol ok!\n");

    find_symbol_from_exec(obj_f);

    arch_create_got(obj_f);

    obj_check_undefineds(obj_f, 1);

    obj_allocate_commons(obj_f);

    image_size = obj_load_size(obj_f);
    INFO("Image size: 0x%x\n", (int)image_size);

    INFO("Relocate start...\n");
    obj_relocate(obj_f, 0x50000000);
    INFO("Relocate success!!!\n");
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