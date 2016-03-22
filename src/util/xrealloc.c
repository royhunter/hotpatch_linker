#include <stdlib.h>
#include "util.h"


/*======================================================================*/

void *
xrealloc(void *old, size_t size)
{
  void *ptr = realloc(old, size);
  if (!ptr)
    {
      ERROR("Out of memory");
      exit(1);
    }
  return ptr;
}

