#include <utils.h>

/*
 * Safely allocates memory.
 */
void *
xalloc (size_t len)
{
  void *mem = malloc (len);
  if (!mem)
    {
      fprintf (stderr, "can't allocated memory");
      exit (1);
    }

  return mem;
}
