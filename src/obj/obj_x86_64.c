#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <obj.h>


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

