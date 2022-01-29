#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

static void   display_sl1     (sl1_t *sl1);
static void   display_ctb     (ctb_t *ctb);

// TODO
/*
 * Display meta data about given sl1 file.
 */
static void
display_sl1 (sl1_t *sl1)
{
}

// TODO
/*
 * Display meta data about given ctb file.
 */
static void
display_ctb (ctb_t *ctb)
{
}

// PUBLIC

/*
 * Inspect given file, showing its type and most relevant
 * information.
 */
void
inspect (const char *in)
{
  sl1_t sl1;
  if (parse_sl1_file (&sl1, in))
    {
      display_sl1 (&sl1);
      exit (0);
    }

  ctb_t ctb;
  if (parse_ctb_file (&ctb, in))
    {
      display_ctb (&ctb);
      exit (0);
    }
  else
    {
      fprintf (stderr, "Unrecognized file at %s\n", in);
      exit (1);
    }
}
