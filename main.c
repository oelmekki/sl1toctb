#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "convert.h"
#include "inspect.h"

typedef struct {
  bool v3;
  bool inspect_mode;
  const char *in;
  char *out;
} options_t;

static void
usage (const char *progname)
{
  printf ("%s [-3h] [--help] <inputfile> [<outputfile>] \n\
%s -i <file> \n\
  \n\
  Convert a sl1 file into a ctb file. \n\
  Output file will have the same name as input file with the \n\
  .sl1 extension replaced with .ctb, unless `outputfile` is \n\
  specified. \n\
  \n\
  By default, outputs ctb version 4 files.\n\
  \n\
  When `-i` option is provided, inspect the given file instead.\n\
  \n\
  Options: \n\
  \n\
  -3        : output ctb version 3 \n\
  -4        : output ctb version 4 \n\
  -i        ; inspect file. \n\
  -h|--help : show this help \n\
  \n", progname, progname);
}

static void
parse_options (options_t *options, const size_t argc, char ** const argv)
{
  options->inspect_mode = false;
  options->v3 = false;
  options->in = NULL;
  options->out = NULL;

  for (size_t i = 1; i < argc; i++)
    {
      if (strncmp (argv[i], "-3", 3) == 0)
        {
          options->v3 = true;
          continue;
        }

      if (strncmp (argv[i], "-4", 3) == 0)
        {
          options->v3 = false;
          continue;
        }

      if (strncmp (argv[i], "-i", 3) == 0)
        {
          options->inspect_mode = true;
          continue;
        }

      if (strncmp (argv[i], "-h", 3) == 0 || strncmp (argv[i], "--help", 7) == 0)
        {
          usage (argv[0]);
          exit (0);
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
  int ret = 0;
  options_t options;
  parse_options (&options, argc, argv);

  if (options.inspect_mode)
    ret = inspect (options.in);
  else
    ret = convert (options.in, options.out, options.v3);

  if (options.out) free (options.out);

  return ret;
}
