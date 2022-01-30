#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "parser.h"


sl1_t *
new_sl1 ()
{
  sl1_t *s = xalloc (sizeof (sl1_t));
  return s;
}

void
free_sl1 (sl1_t *s)
{
  if (!s) return;

  free (s);
}

ctb_t *
new_ctb ()
{
  ctb_t *c = xalloc (sizeof (ctb_t));
  return c;
}

void
free_ctb (ctb_t *c)
{
  if (!c) return;
  if (c->machine_name) free (c->machine_name);
  if (c->disclaimer) free (c->disclaimer);

  free (c);
}

// TODO
/*
 * Read a sl1 file at `in` and initialize the `sl1` struct with it.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
int
parse_sl1_file (sl1_t *sl1, const char *in)
{ 
  return 1; // FIXME writing ctb parser first
}

// TODO
/*
 * Read a ctb file at `in` and initialize the `ctb` struct with it.
 *
 * Returns non-zero in case of error.
 */
int
parse_ctb_file (ctb_t *ctb, const char *in)
{ 
  int ret = 0;
  FILE *file = fopen (in, "r");
  if (!file)
    {
      fprintf (stderr, "parse_ctb_file() : can't open file %s\n", in);
      ret = 1;
      goto cleanup;
    }

  size_t count = fread ((void *) &ctb->headers, sizeof (ctb_headers_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parse_ctb_file() : Error while reading file's headers.\n");
      ret = 1;
      goto cleanup;
    }

  if (ctb->headers.magic != 0x12fd0086)
    {
      fprintf (stderr, "parse_ctb_file() : This is not a ctb file.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->headers.print_config_offset, SEEK_SET);
  count = fread ((void *) &ctb->print_config, sizeof (ctb_print_config_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parse_ctb_file() : Error while reading file's print config.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->headers.slicer_config_offset, SEEK_SET);
  count = fread ((void *) &ctb->slicer_config, sizeof (ctb_slicer_config_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parse_ctb_file() : Error while reading file's slicer config.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->slicer_config.machine_type_offset, SEEK_SET);
  ctb->machine_name = xalloc (ctb->slicer_config.machine_type_len + 1);
  count = fread ((void *) ctb->machine_name, ctb->slicer_config.machine_type_len, 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parse_ctb_file() : Error while reading machine name.\n");
      ret = 1;
      goto cleanup;
    }
  ctb->machine_name[ctb->slicer_config.machine_type_len] = 0;

  if (ctb->headers.version == 4)
    {
      fseek (file, ctb->slicer_config.print_config_v4_offset, SEEK_SET);
      count = fread ((void *) &ctb->print_config_v4, sizeof (ctb_print_config_v4_t), 1, file);
      if (count != 1)
        {
          fprintf (stderr, "parse_ctb_file() : Error while reading file's print config v4.\n");
          ret = 1;
          goto cleanup;
        }

      fseek (file, ctb->print_config_v4.disclaimer_offset, SEEK_SET);
      ctb->disclaimer = xalloc (ctb->print_config_v4.disclaimer_len + 1);
      count = fread ((void *) ctb->disclaimer, ctb->print_config_v4.disclaimer_len, 1, file);
      if (count != 1)
        {
          fprintf (stderr, "parse_ctb_file() : Error while reading disclaimer.\n");
          ret = 1;
          goto cleanup;
        }
      ctb->disclaimer[ctb->print_config_v4.disclaimer_len] = 0;
    }

  fseek (file, ctb->headers.large_preview_offset, SEEK_SET);
  count = fread ((void *) &ctb->large_preview, sizeof (ctb_preview_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parse_ctb_file() : Error while reading file's large preview metadata.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->headers.small_preview_offset, SEEK_SET);
  count = fread ((void *) &ctb->small_preview, sizeof (ctb_preview_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parse_ctb_file() : Error while reading file's small preview metadata.\n");
      ret = 1;
      goto cleanup;
    }

  cleanup:
  if (file) fclose (file);
  return ret;
}
