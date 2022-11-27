#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

/*
 * Safely allocates memory.
 */
void *
xalloc (size_t len)
{
  void *mem = calloc (1, len);
  if (!mem)
    {
      fprintf (stderr, "utils.c: can't allocated memory");
      exit (1);
    }

  return mem;
}
