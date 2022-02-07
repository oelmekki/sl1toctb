#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
  if (c->file_path) free (c->file_path);
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

/*
 * Read a ctb file at `in` and initialize the `ctb` struct with it.
 *
 * Returns non-zero in case of error.
 */
int
parse_ctb_file (ctb_t *ctb, const char *in)
{ 
  int ret = 0;
  ctb->file_path = strdup (in);
  if (!ctb->file_path)
    {
      fprintf (stderr, "Not enough memory.");
      exit (1);
    }

  FILE *file = fopen (in, "rb");
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

// TODO
/*
 * Read preview image for file `ctb` and store its content in `data`.
 *
 * To select which image to preview, pass CTB_PREVIEW_LARGE or
 * CTB_PREVIEW_SMALL as `type`.
 *
 * Returns non-zero in case of error.
 */
int
read_preview_file (u_int8_t **data, size_t *len, const ctb_t *ctb, size_t type)
{
  int err = 0;
  char *compressed = NULL;
  FILE *file = NULL;

  if (!ctb->file_path)
    {
      fprintf (stderr, "read_preview_file() : internal error: the ctb object has no file_path attribute.\n");
      err = 1;
      goto cleanup;
    }

  file = fopen (ctb->file_path, "rb");
  if (!file)
    {
      fprintf (stderr, "read_preview_file() : can't open file %s\n", ctb->file_path);
      err = 1;
      goto cleanup;
    }

  const ctb_preview_t *preview = type == CTB_PREVIEW_LARGE ? &ctb->large_preview : &ctb->small_preview;

  err = fseek (file, preview->image_offset, SEEK_SET);
  if (err)
    {
      fprintf (stderr, "read_preview_file() : can't find preview image in file %s\n", ctb->file_path);
      goto cleanup;
    }

  compressed = xalloc (preview->image_length);
  size_t count = fread (compressed, preview->image_length, 1, file);
  if (count != 1)
    {
      fprintf (stderr, "read_preview_file() : can't read file %s\n", ctb->file_path);
      err = 1;
      goto cleanup;
    }

  *data = xalloc (preview->resolution_x * preview->resolution_y * sizeof (u_int8_t) * 3);
  *len = 0;
  for (size_t i = 0; i < preview->image_length; i++)
    {
      u_int32_t dot = (u_int32_t)((compressed[i] & 0xFF) | ((compressed[i+1] & 0xFF) << 8));
      i++;

      char red = (char)(((dot >> 11) & 0x1F) << 3);
      char green = (char)(((dot >> 6) & 0x1F) << 3);
      char blue = (char)((dot & 0x1F) << 3);
      size_t repeat = 1;
      if ((dot & 0x0020) == 0x0020)
        {
          repeat += (compressed[i+1] & 0xFF) | ((compressed[i+2] & 0x0F) << 8);
          i += 2;
        }

      for (size_t j = 0; j < repeat; j++)
        {
          (*data)[(*len)++] = red;
          (*data)[(*len)++] = green;
          (*data)[(*len)++] = blue;
        }
    }

  cleanup:
  if (file) fclose (file);
  if (compressed) free (compressed);
  return err;
}
