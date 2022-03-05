#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <zip.h>
#include <libgen.h>
#include "utils.h"
#include "parser.h"
#include "spng.h"


void  sl1_set_parsed_attribute  (sl1_t *sl1, const char *attribute, const char *value);
int   sl1_parse_file            (sl1_t *sl1, zip_file_t *file);


void
sl1_set_parsed_attribute (sl1_t *sl1, const char *attribute, const char *value)
{
  if (strncmp (attribute, "bed_shape", 100) == 0)
    sl1->bed_shape = strdup (value);

  if (strncmp (attribute, "default_sla_material_profile", 100) == 0)
    sl1->default_sla_material_profile = strdup (value);

  if (strncmp (attribute, "default_sla_print_profile", 100) == 0)
    sl1->default_sla_print_profile = strdup (value);

  if (strncmp (attribute, "printer_notes", 100) == 0)
    sl1->printer_notes = strdup (value);

  if (strncmp (attribute, "printer_settings_id", 100) == 0)
    sl1->printer_settings_id = strdup (value);

  if (strncmp (attribute, "sla_material_settings_id", 100) == 0)
    sl1->sla_material_settings_id = strdup (value);

  if (strncmp (attribute, "sla_print_settings_id", 100) == 0)
    sl1->sla_print_settings_id = strdup (value);


  if (strncmp (attribute, "bottle_cost", 100) == 0)
    sl1->bottle_cost = atof (value);

  if (strncmp (attribute, "bottle_volume", 100) == 0)
    sl1->bottle_volume = atof (value);

  if (strncmp (attribute, "bottle_weight", 100) == 0)
    sl1->bottle_weight = atof (value);

  if (strncmp (attribute, "display_height", 100) == 0)
    sl1->display_height = atof (value);

  if (strncmp (attribute, "exposure_time", 100) == 0)
    sl1->exposure_time = atof (value);

  if (strncmp (attribute, "initial_layer_height", 100) == 0)
    sl1->initial_layer_height = atof (value);

  if (strncmp (attribute, "layer_height", 100) == 0)
    sl1->layer_height = atof (value);

  if (strncmp (attribute, "max_print_height", 100) == 0)
    sl1->max_print_height = atof (value);


  if (strncmp (attribute, "display_orientation", 100) == 0)
    sl1->display_orientation = atoi (value);

  if (strncmp (attribute, "display_pixels_x", 100) == 0)
    sl1->display_pixels_x = atoi (value);

  if (strncmp (attribute, "display_pixels_y", 100) == 0)
    sl1->display_pixels_y = atoi (value);

  if (strncmp (attribute, "display_width", 100) == 0)
    sl1->display_width = atof (value);

  if (strncmp (attribute, "faded_layers", 100) == 0)
    sl1->faded_layers = atoi (value);

  if (strncmp (attribute, "fast_tilt_time", 100) == 0)
    sl1->fast_tilt_time = atoi (value);

  if (strncmp (attribute, "initial_exposure_time", 100) == 0)
    sl1->initial_exposure_time = atoi (value);

  if (strncmp (attribute, "numFast", 100) == 0)
    sl1->num_fast_layers = atoi (value);

  if (strncmp (attribute, "numSlow", 100) == 0)
    sl1->num_slow_layers = atoi (value);

  if (strncmp (attribute, "printTime", 100) == 0)
    sl1->print_time = atoi (value);


  if (strncmp (attribute, "display_mirror_x", 100) == 0)
    sl1->display_mirror_x = atoi (value);

  if (strncmp (attribute, "display_mirror_y", 100) == 0)
    sl1->display_mirror_y = atoi (value);
}

int
sl1_parse_file (sl1_t *sl1, zip_file_t *file)
{
  int err = 0;
  char content[1000001] = "";

  int count = zip_fread (file, content, 1000000);
  if (count == -1)
    {
      fprintf (stderr, "parser.c: sl1_parse_file() : can't read prusaslicer.ini in the sl1 file.");
      err = 1;
      goto cleanup;
    }

  content[count] = 0;
  char line[5001] = "";
  size_t line_i = 0;

  for (int i = 0; i < count; i++)
    {
      if (content[i] == '\n' || content[i] == '\r')
        {
          line[line_i] = 0;

          char attribute[5001] = "";
          char value[5001] = "";
          int split_pos = 0;

          for (size_t j = 0; j < line_i; j++)
            {
              if (line[j] == ' ')
                {
                  split_pos = j;
                  attribute[j] = 0;
                  break;
                }

              attribute[j] = line[j];
            }

          split_pos += 3; // move over the ' = '

          for (size_t j = split_pos; j < line_i; j++)
            value[j-split_pos] = line[j];

          sl1_set_parsed_attribute (sl1, attribute, value);

          line[0] = 0;
          line_i = 0;
          continue;
        }

      line[line_i++] = content[i];
      if (line_i == 4999)
        {
          line[5000] = 0;
          fprintf (stderr, "parser.c: sl1_parse_file() : Line is way too long : %s\n", line);
          err = 1;
          goto cleanup;
        }
    }

  cleanup:
  return err;
}


// PUBLIC

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

  if (s->file_path) free (s->file_path);
  if (s->base_name) free (s->base_name);
  if (s->bed_shape) free (s->bed_shape);
  if (s->default_sla_material_profile) free (s->default_sla_material_profile);
  if (s->default_sla_print_profile) free (s->default_sla_print_profile);
  if (s->printer_notes) free (s->printer_notes);
  if (s->printer_settings_id) free (s->printer_settings_id);
  if (s->sla_material_settings_id) free (s->sla_material_settings_id);
  if (s->sla_print_settings_id) free (s->sla_print_settings_id);

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
  if (c->v4_disclaimer) free (c->v4_disclaimer);

  free (c);
}

/*
 * Tells if giving path is a sl1 file.
 */
bool
is_sl1_file (const char *in)
{
  zip_t *archive = NULL;
  int err = 0;
  bool sl1_file = false;

  archive = zip_open (in, ZIP_RDONLY, &err);
  if (err)
    goto cleanup;

  if (zip_name_locate (archive, "prusaslicer.ini", 0) == -1)
    goto cleanup;

  if (zip_name_locate (archive, "config.ini", 0) == -1)
    goto cleanup;

  sl1_file = true;
    
  cleanup:
  if (archive) zip_close (archive);
  return sl1_file;
}

/*
 * Tells if giving path is a ctb file.
 */
bool
is_ctb_file (const char *in)
{
  bool is_ctb = false;
  ctb_t *ctb = NULL;
  FILE *file = fopen (in, "rb");
  if (!file)
    {
      fprintf (stderr, "parser.c: is_ctb_file() : can't open file %s\n", in);
      goto cleanup;
    }

  ctb = new_ctb ();

  size_t count = fread ((void *) &ctb->headers, sizeof (ctb_headers_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parser.c: is_ctb_file() : Error while reading file's headers.\n");
      goto cleanup;
    }

  if (ctb->headers.magic == 0x12fd0086)
    is_ctb = true;

  cleanup:
  if (file) fclose (file);
  if (ctb) free_ctb (ctb);
  return is_ctb;
}

/*
 * Read a sl1 file at `in` and initialize the `sl1` struct with it.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
int
parse_sl1_archive (sl1_t *sl1, const char *in)
{ 
  zip_t *archive = NULL;
  zip_file_t *file = NULL;
  int err = 0;

  sl1->file_path = strdup (in);
  char *base_name = basename (sl1->file_path);
  sl1->base_name = xalloc (strlen (base_name));
  for (size_t i = 0; i < strlen (base_name) - 4; i++) // 4 == .sl1
    sl1->base_name[i] = base_name[i];

  if (!is_sl1_file (in))
    {
      fprintf (stderr, "parser.c: parse_sl1_archive() : this is not a sl1 file.\n");
      err = 1;
      goto cleanup;
    }

  archive = zip_open (in, ZIP_RDONLY, &err);
  if (err)
    {
      fprintf (stderr, "parser.c: parse_sl1_archive() : can't open sl1 archive.\n");
      goto cleanup;
    }

  file = zip_fopen (archive, "prusaslicer.ini", 0);
  if (!file)
    {
      fprintf (stderr, "parser.c: parse_sl1_archive() : can't open prusaslicer.ini in the sl1 file.");
      err = 1;
      goto cleanup;
    }

  err = sl1_parse_file (sl1, file);
  if (err)
    {
      fprintf (stderr, "parser.c: parse_sl1_archive() : can't parse prusaslicer.ini.");
      goto cleanup;
    }

  zip_fclose (file);

  file = zip_fopen (archive, "config.ini", 0);
  if (!file)
    {
      fprintf (stderr, "parser.c: parse_sl1_archive() : can't open config.ini in the sl1 file.");
      err = 1;
      goto cleanup;
    }

  err = sl1_parse_file (sl1, file);
  if (err)
    {
      fprintf (stderr, "parser.c: parse_sl1_archive() : can't parse config.ini.");
      goto cleanup;
    }

  cleanup:
  if (file) zip_fclose (file);
  if (archive) zip_close (archive);
  return err;
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
      fprintf (stderr, "parser.c: Not enough memory.");
      exit (1);
    }

  FILE *file = fopen (in, "rb");
  if (!file)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : can't open file %s\n", in);
      ret = 1;
      goto cleanup;
    }

  size_t count = fread ((void *) &ctb->headers, sizeof (ctb_headers_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading file's headers.\n");
      ret = 1;
      goto cleanup;
    }

  if (ctb->headers.magic != 0x12fd0086)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : This is not a ctb file.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->headers.print_config_offset, SEEK_SET);
  count = fread ((void *) &ctb->print_config, sizeof (ctb_print_config_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading file's print config.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->headers.slicer_config_offset, SEEK_SET);
  count = fread ((void *) &ctb->slicer_config, sizeof (ctb_slicer_config_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading file's slicer config.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->slicer_config.machine_type_offset, SEEK_SET);
  ctb->machine_name = xalloc (ctb->slicer_config.machine_type_len + 1);
  count = fread ((void *) ctb->machine_name, ctb->slicer_config.machine_type_len, 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading machine name.\n");
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
          fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading file's print config v4.\n");
          ret = 1;
          goto cleanup;
        }

      fseek (file, ctb->print_config_v4.disclaimer_offset, SEEK_SET);
      ctb->v4_disclaimer = xalloc (ctb->print_config_v4.disclaimer_len + 1);
      count = fread ((void *) ctb->v4_disclaimer, ctb->print_config_v4.disclaimer_len, 1, file);
      if (count != 1)
        {
          fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading disclaimer.\n");
          ret = 1;
          goto cleanup;
        }
      ctb->v4_disclaimer[ctb->print_config_v4.disclaimer_len] = 0;
    }

  fseek (file, ctb->headers.large_preview_offset, SEEK_SET);
  count = fread ((void *) &ctb->large_preview, sizeof (ctb_preview_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading file's large preview metadata.\n");
      ret = 1;
      goto cleanup;
    }

  fseek (file, ctb->headers.small_preview_offset, SEEK_SET);
  count = fread ((void *) &ctb->small_preview, sizeof (ctb_preview_t), 1, file);
  if (count != 1)
    {
      fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading file's small preview metadata.\n");
      ret = 1;
      goto cleanup;
    }

  ctb->layer_headers = xalloc (sizeof (ctb_layer_header_t) * ctb->headers.layer_count);
  fseek (file, ctb->headers.layer_table_offset, SEEK_SET);
  for (size_t i = 0; i < ctb->headers.layer_count; i++)
    {
      count = fread ((void *) &ctb->layer_headers[i].base, sizeof (ctb_layer_header_base_t), 1, file);
      if (count != 1)
        {
          fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading layer base metadata.\n");
          ret = 1;
          goto cleanup;
        }
    }

  for (size_t i = 0; i < ctb->headers.layer_count; i++)
    {
      // extended metadata are 84 bytes before the mentioned address of the layer
      fseek (file, ctb->layer_headers[i].base.data_offset - 84, SEEK_SET);
      count = fread ((void *) &ctb->layer_headers[i].extended, sizeof (ctb_layer_header_extended_t), 1, file);
      if (count != 1)
        {
          fprintf (stderr, "parser.c: parse_ctb_file() : Error while reading layer extended metadata.\n");
          ret = 1;
          goto cleanup;
        }
    }

  cleanup:
  if (file) fclose (file);
  return ret;
}

/*
 * Read preview image for file `ctb` and store its content in `data`.
 *
 * `data` is an uncompressed bitmap.
 *
 * To select which image to preview, pass PREVIEW_LARGE or
 * PREVIEW_SMALL as `type`.
 *
 * Returns non-zero in case of error.
 */
int
ctb_read_preview_file (uint8_t **data, size_t *len, const ctb_t *ctb, size_t type)
{
  int err = 0;
  char *compressed = NULL;
  FILE *file = NULL;

  if (!ctb->file_path)
    {
      fprintf (stderr, "parser.c: ctb_read_preview_file() : internal error: the ctb object has no file_path attribute.\n");
      err = 1;
      goto cleanup;
    }

  file = fopen (ctb->file_path, "rb");
  if (!file)
    {
      fprintf (stderr, "parser.c: ctb_read_preview_file() : can't open file %s\n", ctb->file_path);
      err = 1;
      goto cleanup;
    }

  const ctb_preview_t *preview = type == PREVIEW_LARGE ? &ctb->large_preview : &ctb->small_preview;

  err = fseek (file, preview->image_offset, SEEK_SET);
  if (err)
    {
      fprintf (stderr, "parser.c: ctb_read_preview_file() : can't find preview image in file %s\n", ctb->file_path);
      goto cleanup;
    }

  compressed = xalloc (preview->image_length);
  size_t count = fread (compressed, 1, preview->image_length, file);
  if (count != preview->image_length)
    {
      fprintf (stderr, "parser.c: ctb_read_preview_file() : can't read file %s\n", ctb->file_path);
      err = 1;
      goto cleanup;
    }

  *data = xalloc (preview->resolution_x * preview->resolution_y * sizeof (uint8_t) * 3);
  *len = 0;
  for (size_t i = 0; i < preview->image_length; i++)
    {
      uint32_t dot = (uint32_t)((compressed[i] & 0xFF) | ((compressed[i+1] & 0xFF) << 8));
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

/*
 * Read preview image for file `sl1` and store its content in `data`.
 *
 * `data` is an uncompressed bitmap.
 *
 * To select which image to preview, pass PREVIEW_LARGE or
 * PREVIEW_SMALL as `type`.
 *
 * Returns non-zero in case of error.
 */
int
sl1_read_preview_file (uint8_t **data, size_t *len, const sl1_t *sl1, size_t type)
{
  int err = 0;
  zip_t *archive = NULL;
  zip_file_t *file = NULL;
  spng_ctx *ctx = NULL;
  uint8_t *png = NULL;

  archive = zip_open (sl1->file_path, ZIP_RDONLY, &err);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't open sl1 archive.\n");
      goto cleanup;
    }

  const char *filename = type == PREVIEW_LARGE ? "thumbnail/thumbnail800x480.png" : "thumbnail/thumbnail400x400.png";

  zip_stat_t info;
  err = zip_stat (archive, filename, 0, &info);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't stat preview file.\n");
      goto cleanup;
    }

  file = zip_fopen (archive, filename, 0);
  if (!file)
    {
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't open thumbnail file in the sl1 file.");
      err = 1;
      goto cleanup;
    }

  size_t png_len = info.size;
  png = xalloc (png_len);

  int count = zip_fread (file, png, png_len);
  if (count < 0 || (size_t) count != png_len)
    {
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't read png file.");
      err = 1;
      goto cleanup;
    }

  ctx = spng_ctx_new (0);
  if (!ctx)
    {
      err = 1;
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't create spng context.\n");
      goto cleanup;
    }

  err = spng_set_png_buffer (ctx, png, png_len);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't set input buffer in spng.\n");
      goto cleanup;
    }

  struct spng_ihdr headers;
  err = spng_get_ihdr (ctx, &headers);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't set parse headers.\n");
      goto cleanup;
    }

  if (type == PREVIEW_LARGE && (headers.width != 800 || headers.height != 480))
    {
      err = 1;
      fprintf (stderr, "parser.c: sl1_read_preview_file() : Large preview image in sl1 file is not 800x480.\n");
      goto cleanup;
    }

  if (type == PREVIEW_SMALL && (headers.width != 400 || headers.height != 400))
    {
      err = 1;
      fprintf (stderr, "parser.c: sl1_read_preview_file() : Small preview image in sl1 file is not 400x400.\n");
      goto cleanup;
    }

  if (headers.color_type != SPNG_COLOR_TYPE_TRUECOLOR_ALPHA)
    {
      err = 1;
      fprintf (stderr, "parser.c: sl1_read_preview_file() : Preview image in sl1 file is not trucolor with alpha channel.\n");
      goto cleanup;
    }

  *len = type == PREVIEW_LARGE ? 800 * 480 * 4 : 400 * 400 * 4;
  *data = xalloc (*len);

  err = spng_decode_image (ctx, (void *) *data, *len, SPNG_FMT_PNG, 0);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_preview_file() : can't decode image.\n");
      goto cleanup;
    }

  cleanup:
  if (file) zip_fclose (file);
  if (archive) zip_close (archive);
  if (ctx) spng_ctx_free (ctx);
  if (png) free (png);
  return err;
}

/*
 * Read image for layer `layer_index` in file `sl1` and store its content in `data`.
 *
 * `data` is an uncompressed bitmap.
 *
 * Returns non-zero in case of error.
 */
int
sl1_read_layer_image_file (uint8_t **data, size_t *len, const sl1_t *sl1, size_t layer_index)
{
  int err = 0;
  zip_t *archive = NULL;
  zip_file_t *file = NULL;
  spng_ctx *ctx = NULL;
  uint8_t *png = NULL;


  archive = zip_open (sl1->file_path, ZIP_RDONLY, &err);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't open sl1 archive.\n");
      goto cleanup;
    }

  char filename[100] = {0};
  snprintf (filename, 99, "%s%05ld.png", sl1->base_name, layer_index);

  zip_stat_t info;
  err = zip_stat (archive, filename, 0, &info);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't stat layer file.\n");
      goto cleanup;
    }

  file = zip_fopen (archive, filename, 0);
  if (!file)
    {
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't open layer file in the sl1 file.");
      err = 1;
      goto cleanup;
    }

  size_t png_len = info.size;
  png = xalloc (png_len);

  int count = zip_fread (file, png, png_len);
  if (count < 0 || (size_t) count != png_len)
    {
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't read png file.");
      err = 1;
      goto cleanup;
    }

  ctx = spng_ctx_new (0);
  if (!ctx)
    {
      err = 1;
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't create spng context.\n");
      goto cleanup;
    }

  err = spng_set_png_buffer (ctx, png, png_len);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't set input buffer in spng.\n");
      goto cleanup;
    }

  struct spng_ihdr headers;
  err = spng_get_ihdr (ctx, &headers);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't parse headers.\n");
      goto cleanup;
    }

  if (headers.width != (uint32_t) sl1->display_pixels_x || headers.height != (uint32_t) sl1->display_pixels_y)
    {
      err = 1;
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : Layer image in sl1 file is not the expected size (%dx%d).\n", sl1->display_pixels_x, sl1->display_pixels_y);
      goto cleanup;
    }

  if (headers.color_type != SPNG_COLOR_TYPE_GRAYSCALE)
    {
      err = 1;
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : Preview image in sl1 file is not greyscale.\n");
      goto cleanup;
    }

  *len = sl1->display_pixels_x * sl1->display_pixels_y;
  *data = xalloc (*len);

  err = spng_decode_image (ctx, (void *) *data, *len, SPNG_FMT_G8, 0);
  if (err)
    {
      fprintf (stderr, "parser.c: sl1_read_layer_image_file() : can't decode image.\n");
      goto cleanup;
    }

  cleanup:
  if (file) zip_fclose (file);
  if (archive) zip_close (archive);
  if (ctx) spng_ctx_free (ctx);
  if (png) free (png);

  return err;
}

/*
 * Decrypt layer data in `raw_data` with key in `encryption_key`.
 *
 * Data in `raw_data` is edited in place.
 */
void
decrypt_layer (uint8_t *raw_data, uint32_t encryption_key, uint32_t layer_index, size_t len)
{
  if (!encryption_key)
    return;

  uint32_t init = encryption_key * 0x2d83cdac + 0xd8a83423;
  uint32_t key = (layer_index * 0x1e1530cd + 0xec3d47cd) * init;
  int index = 0;

  for (size_t i = 0; i < len; i++)
    {
      uint8_t k = (uint8_t) (key >> (8 * index));
      index++;

      if ((index & 3) == 0)
        {
          key += init;
          index = 0;
        }

      raw_data[i] = (uint8_t) (raw_data[i] ^ k);
    }
}

/*
 * Decode the RLE of layer from `raw_data` to `data`.
 *
 * `data` memory will be allocated, you're responsible to free it.
 *
 * The length of decoded data will be stored in `data_len`.
 *
 * Return non-zero in case of error.
 */
int
decode_layer (uint8_t **data, size_t *data_len, const uint8_t *raw_data, size_t raw_len, const ctb_t *ctb, size_t *nonzero_pixels_count)
{
  *data = xalloc (ctb->headers.resolution_x * ctb->headers.resolution_y);
  *data_len = 0;

  for (size_t i = 0; i < raw_len; i++)
    {
      uint8_t code = raw_data[i];
      int stride = 1;

      if ((code & 0x80) == 0x80)
        {
          code &= 0x7f; // extract run length
          i++;

          int stride_len = raw_data[i];

          if ((stride_len & 0x80) == 0)
            stride = stride_len;
          else if ((stride_len & 0xc0) == 0x80)
            {
              stride = ((stride_len & 0x3f) << 8) + raw_data[i + 1];
              i++;
            }
          else if ((stride_len & 0xe0) == 0xc0)
            {
              stride = ((stride_len & 0x1f) << 16) + (raw_data[i + 1] << 8) + raw_data[i + 2];
              i += 2;
            }
          else if ((stride_len & 0xf0) == 0xe0)
            {
              stride = ((stride_len & 0xf) << 24) + (raw_data[i + 1] << 16) + (raw_data[i + 2] << 8) + raw_data[i + 3];
              i += 3;
            }
          else
            {
              fprintf (stderr, "parser.c: decode_layer() : bogus layer data.\n");
              return 1;
            }
        }

      // extend from 7 bits to 8 bits
      if (code != 0)
        {
          code = (code << 1) | 1;
          (*nonzero_pixels_count)++;
        }

      for (int j = 0; j < stride; j++)
        (*data)[(*data_len)++] = code;
    }

  return 0;
}
