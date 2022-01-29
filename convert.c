#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

static int    generate_ctb    (ctb_t *ctb, const sl1_t *sl1, const int version);
static int    write_file      (const ctb_t *ctb, const char *out);

// TODO
/*
 * Generate a ctb struct from a sl1 struct.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
generate_ctb (ctb_t *ctb, const sl1_t *sl1, const int version)
{
  return 0;
}

// TODO
/*
 * Write the provided `ctb` struct to file at `out`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_file (const ctb_t *ctb, const char *out)
{
  return 0;
}

// PUBLIC

// TODO
/*
 * Convert the sl1 file at `in` into a ctb file at `out`.
 *
 * It will be a ctb version 4 file, unless the `v3` flag
 * is set.
 */
void
convert (const char *in, const char *out, const bool v3)
{
  sl1_t sl1;
  ctb_t ctb;

  if (parse_sl1_file (&sl1, in))
    {
      fprintf (stderr, "Can't parse input file.\n");
      exit (1);
    }

  if (generate_ctb (&ctb, &sl1, v3 ? 3 : 4))
    {
      fprintf (stderr, "Can't generate ctb file.\n");
      exit (1);
    }

  if (write_file (&ctb, out))
    {
      fprintf (stderr, "Can't write output file.\n");
      exit (1);
    }
}
