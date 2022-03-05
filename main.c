#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "convert.h"
#include "inspect.h"
#include "parser.h"

typedef struct {
  bool inspect_mode;
  bool show_large_preview_mode;
  bool show_small_preview_mode;
  bool export_layer_data;
  const char *export_dir;
  const char *in;
  char *out;
} options_t;

static void
usage (const char *progname)
{
  printf ("%s [-h] [--help] <input sl1 file> [<output ctb file>] \n\
%s <-i> <ctb file> \n\
%s <-l|-s|-e> <dir> <ctb file> \n\
  \n\
  Convert a sl1 file into a ctb v4 file. \n\
  Output file will have the same name as input file with the \n\
  .sl1 extension replaced with .ctb, unless `output ctb file` is \n\
  specified. \n\
  \n\
  When `-i` option is provided, inspect the given file instead.\n\
  This works with both ctb files and sl1 files.\n\
  \n\
  When `-l` or `-s` option is provided, export preview image instead.\n\
  `-l` exports the large preview, and `-s` exports the small preview.\n\
  The file will be saved in `dir` and will be named `preview.png`.\n\
  Only works for ctb files.\n\
  \n\
  When -e option is provided, export layers and their headers in `dir`.\n\
  Only works for ctb files.\n\
  \n\
  Options: \n\
  \n\
  -i        : inspect file. (ctb or sl1) \n\
  -l        : export large preview. (ctb only) \n\
  -s        : export small preview. (ctb only) \n\
  -e        : export layer data to <dir> (ctb only) \n\
  -h|--help : show this help \n\
  \n", progname, progname, progname);
}

static void
parse_options (options_t *options, const size_t argc, char ** const argv)
{
  for (size_t i = 1; i < argc; i++)
    {
      if (strncmp (argv[i], "-i", 3) == 0)
        {
          options->inspect_mode = true;
          continue;
        }

      if (strncmp (argv[i], "-l", 3) == 0)
        {
          if (argc < i+2)
            {
              usage (argv[0]);
              exit (1);
            }

          options->show_large_preview_mode = true;
          options->export_dir = argv[++i];
          continue;
        }

      if (strncmp (argv[i], "-s", 3) == 0)
        {
          if (argc < i+2)
            {
              usage (argv[0]);
              exit (1);
            }

          options->show_small_preview_mode = true;
          options->export_dir = argv[++i];
          continue;
        }

      if (strncmp (argv[i], "-h", 3) == 0 || strncmp (argv[i], "--help", 7) == 0)
        {
          usage (argv[0]);
          exit (0);
        }

      if (strncmp (argv[i], "-e", 3) == 0)
        {
          if (argc < i+2)
            {
              usage (argv[0]);
              exit (1);
            }

          options->export_layer_data = true;
          options->export_dir = argv[++i];
          continue;
        }

      if (!options->in)
        {
          options->in = argv[i];
          continue;
        }

      if (!options->out)
        {
          options->out = strdup (argv[i]);
          continue;
        }
    }

  if (!options->in)
    {
      usage (argv[0]);
      exit (1);
    }

  if (!options->out)
    {
      size_t len = strlen (options->in);
      char *ptr = (char *) options->in;
      ptr += len - 4;
      if (strncmp (ptr, ".sl1", 5) == 0)
        {
          options->out = strdup (options->in);
          options->out[len-3] = 'c';
          options->out[len-2] = 't';
          options->out[len-1] = 'b';
        }
      else
        {
          options->out = xalloc (len + 5);
          snprintf (options->out, len + 5, "%s.ctb", options->in);
        }
    }
}

int
main (int argc, char **argv)
{
  int err = 0;
  options_t *options = xalloc (sizeof (options_t));
  parse_options (options, argc, argv);

  if (options->inspect_mode)
    {
      err = inspect (options->in);
      goto cleanup;
    }

  if (options->show_large_preview_mode)
    {
      err = show_preview_image (options->in, PREVIEW_LARGE, options->export_dir);
      goto cleanup;
    }

  if (options->show_small_preview_mode)
    {
      err = show_preview_image (options->in, PREVIEW_SMALL, options->export_dir);
      goto cleanup;
    }

  if (options->export_layer_data)
    {
      err = export_layers (options->in, options->export_dir);
      goto cleanup;
    }

  if (strncmp (options->in, options->out, 1000) == 0)
    {
      fprintf (stderr, "main.c: Please specify an output file different from input file.\n\n");
      usage (argv[0]);
      exit (1);
    }

  err = convert (options->in, options->out);

  cleanup:
  if (options->out) free (options->out);
  free (options);
  return err;
}
