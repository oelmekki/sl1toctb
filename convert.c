#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "parser.h"
#include "utils.h"

#define RLE16_ENCODING_LIMIT 0xFFF

static int  write_file            (const sl1_t *sl1, const char *out, bool v3);
static int  write_headers         (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_large_preview   (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_small_preview   (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_print_config    (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_slicer_config   (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_machine_name    (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_v4_disclaimer   (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_v4_config       (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  write_layers          (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3);
static int  encode_rle15          (uint8_t **out_buffer, size_t *out_buffer_size, uint8_t *in_buffer, size_t in_buffer_size);
static int  encode_rle1           (uint8_t **encoded_buffer, size_t *encoded_buffer_size, const uint8_t *in_buffer,
                                   size_t in_buffer_size);
static int  encrypt_layer         (uint8_t **out_buffer, size_t *out_buffer_size, const uint8_t *encoded_buffer,
                                   size_t encoded_buffer_size, uint32_t seed, uint32_t layer_index);

/*
 * Write the provided `sl1` struct to ctb file at `out`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_file (const sl1_t *sl1, const char *out, bool v3)
{
  int err = 0;
  FILE *file = NULL;
  ctb_t *ctb = new_ctb ();
  size_t pos = 0;

  file = fopen (out, "wb");
  if (!file)
    {
      fprintf (stderr, "convert.c: write_file() : can't open file for writing : %s\n", out);
      err = 1;
      goto cleanup;
    }

  // preallocate memory for headers
  int count = fwrite (&ctb->headers, 1, sizeof (ctb->headers), file);
  if ((size_t) count != sizeof (ctb->headers))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_file(): error while allocating headers memory in file.\n");
      goto cleanup;
    }
  pos += sizeof (ctb->headers);

  err = write_large_preview (file, &pos, ctb, sl1, v3);
  if (err)
    {
      fprintf (stderr, "convert.c: write_file() : can't write large preview\n");
      err = 1;
      goto cleanup;
    }

  err = write_small_preview (file, &pos, ctb, sl1, v3);
  if (err)
    {
      fprintf (stderr, "convert.c: write_file() : can't write small preview\n");
      err = 1;
      goto cleanup;
    }

  err = write_print_config (file, &pos, ctb, sl1, v3);
  if (err)
    {
      fprintf (stderr, "convert.c: write_file() : can't write print config\n");
      err = 1;
      goto cleanup;
    }

  ctb->headers.slicer_config_offset = pos;

  // preallocate memory for slicer_config
  count = fwrite (&ctb->slicer_config, 1, sizeof (ctb->slicer_config), file);
  if ((size_t) count != sizeof (ctb->slicer_config))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_file(): error while allocating slicer config memory in file.\n");
      goto cleanup;
    }
  pos += sizeof (ctb->slicer_config);

  err = write_machine_name (file, &pos, ctb, sl1, v3);
  if (err)
    {
      fprintf (stderr, "convert.c: write_file() : can't write machine name\n");
      err = 1;
      goto cleanup;
    }

  if (!v3)
    {
      err = write_v4_disclaimer (file, &pos, ctb, sl1, v3);
      if (err)
        {
          fprintf (stderr, "convert.c: write_file() : can't write v4 disclaimer\n");
          err = 1;
          goto cleanup;
        }

      err = write_v4_config (file, &pos, ctb, sl1, v3);
      if (err)
        {
          fprintf (stderr, "convert.c: write_file() : can't write v4 config\n");
          err = 1;
          goto cleanup;
        }
    }

  err = write_layers (file, &pos, ctb, sl1, v3);
  if (err)
    {
      fprintf (stderr, "convert.c: write_file() : can't write layers\n");
      err = 1;
      goto cleanup;
    }

  fseek (file, ctb->headers.slicer_config_offset, SEEK_SET);
  err = write_slicer_config (file, &pos, ctb, sl1, v3);
  if (err)
    {
      fprintf (stderr, "convert.c: write_file() : can't write slicer config\n");
      err = 1;
      goto cleanup;
    }

  fseek (file, 0, SEEK_SET);
  err = write_headers (file, &pos, ctb, sl1, v3);
  if (err)
    {
      fprintf (stderr, "convert.c: write_file() : can't write headers\n");
      err = 1;
      goto cleanup;
    }


  cleanup:
  free_ctb (ctb);
  if (file) fclose (file);
  return err;
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_headers (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;

  ctb->headers.magic = 0x12fd0086;
  ctb->headers.version = v3 ? 3 : 4;
  ctb->headers.bed_size_x = sl1->display_orientation == SL1_ORIENTATION_LANDSCAPE ? sl1->display_width : sl1->display_height;
  ctb->headers.bed_size_y = sl1->display_orientation == SL1_ORIENTATION_LANDSCAPE ? sl1->display_height : sl1->display_width;
  ctb->headers.bed_size_z = sl1->max_print_height;
  ctb->headers.total_height = (double) sl1->num_fast_layers * sl1->layer_height + (double) sl1->num_slow_layers * sl1->layer_height;
  ctb->headers.layer_height = sl1->layer_height;
  ctb->headers.layer_exposure = sl1->exposure_time;
  ctb->headers.bottom_exposure = sl1->initial_exposure_time;
  ctb->headers.light_off_delay = 0;
  ctb->headers.bottom_layer_count = sl1->faded_layers;
  ctb->headers.resolution_x = sl1->display_orientation == SL1_ORIENTATION_LANDSCAPE ? sl1->display_pixels_x : sl1->display_pixels_y;
  ctb->headers.resolution_y = sl1->display_orientation == SL1_ORIENTATION_LANDSCAPE ? sl1->display_pixels_y : sl1->display_pixels_x;
  ctb->headers.layer_count = sl1->num_fast_layers + sl1->num_slow_layers;
  ctb->headers.print_time = sl1->print_time;
  ctb->headers.projection = 1;
  ctb->headers.antialias_level = 1;
  ctb->headers.pwm_level = 255;
  ctb->headers.bottom_pwm_level = 255;

  int count = fwrite (&ctb->headers, 1, sizeof (ctb->headers), file);
  if ((size_t) count != sizeof (ctb->headers))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_headers(): error while writing to file.\n");
      goto cleanup;
    }

  (*index) += sizeof (ctb->headers);

  cleanup:
  return err;
}

/*
 * Encode a pixel in the rle15 format.
 *
 * It means that it encodes the pixel on 1 byte, with a possible run flag on
 * the lsb of the green channel, and if that flag is set, the next byte
 * encodes how many times that pixel is repeated.
 */
void
rle15_add_pixel (uint8_t *tmp_buffer, uint16_t color15, uint32_t repetition, size_t *out_buffer_size)
{
  uint8_t REPEAT_RGB15_MASK = 0x20;

  switch (repetition)
    {
      case 0:
        return;

      case 1:
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) (color15 & ~REPEAT_RGB15_MASK);
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) ((color15 & ~REPEAT_RGB15_MASK) >> 8);
        break;

      case 2:
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) (color15 & ~REPEAT_RGB15_MASK);
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) ((color15 & ~REPEAT_RGB15_MASK) >> 8);
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) (color15 & ~REPEAT_RGB15_MASK);
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) ((color15 & ~REPEAT_RGB15_MASK) >> 8);
        break;

      default:
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) (color15 | REPEAT_RGB15_MASK);
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) ((color15 | REPEAT_RGB15_MASK) >> 8);
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) ((repetition - 1) | 0x3000);
        tmp_buffer[(*out_buffer_size)++] = (uint8_t) (((repetition - 1) | 0x3000) >> 8);
        break;
    }
}

/*
 * Encode raw bitmap `in_buffer` of length `in_buffer_size` to
 * the RLE15 encoding of ctb preview images in `out_buffer`.
 *
 * `out_buffer_size` will be set to the length of `out_buffer`.
 *
 * Returns non-zero in case of error.
 */
static int
encode_rle15 (uint8_t **out_buffer, size_t *out_buffer_size, uint8_t *in_buffer, size_t in_buffer_size)
{
  int err = 0;
  uint8_t *tmp_buffer = xalloc (in_buffer_size);
  uint16_t color15 = 0; 
  uint32_t repetition = 0;
  (*out_buffer_size) = 0;

  for (size_t i = 0; i < in_buffer_size; i++)
    {
      uint16_t new_color15 = (in_buffer[i+2] >> 3) | ((in_buffer[i+1] >> 2) << 5) | ((in_buffer[i] >> 3) << 11);
      i += 2;

      if (new_color15 == color15)
        {
          repetition++;
          if (repetition == RLE16_ENCODING_LIMIT)
            {
              rle15_add_pixel (tmp_buffer, color15, repetition, out_buffer_size);
              repetition = 0;
            }
        }
      else
        {
          rle15_add_pixel (tmp_buffer, color15, repetition, out_buffer_size);
          color15 = new_color15;
          repetition = 1;
        }
    }

  rle15_add_pixel (tmp_buffer, color15, repetition, out_buffer_size);

  *out_buffer = xalloc (*out_buffer_size);
  memcpy (*out_buffer, tmp_buffer, *out_buffer_size);

  if (tmp_buffer) free (tmp_buffer);
  return err;
}

/*
 * Write the large preview in `file`, filling `ctb` by using `sl1`.
 *
 * The preview images in sl1 and ctb do not have the same size nor
 * aspect ratio, so we need to crop and resize them. The large preview
 * is converted from 800x480 to 400x300.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_large_preview (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;
  uint8_t *in_buffer = NULL;
  size_t in_buffer_size = 0;
  uint8_t *transform_buffer = NULL;
  size_t transform_buffer_size = 0;
  uint8_t *out_buffer = NULL;
  size_t out_buffer_size = 0;
  ctb_preview_t *headers = NULL;

  ctb->headers.large_preview_offset = *index;

  headers = xalloc (sizeof (ctb_preview_t));

  int count = fwrite (headers, 1, sizeof (ctb_preview_t), file);
  if ((size_t) count != sizeof (ctb_preview_t))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_large_preview(): error allocating memory for preview headers in file.\n");
      goto cleanup;
    }
  *index += sizeof (ctb_preview_t);
  headers->image_offset = *index;

  err = sl1_read_preview_file (&in_buffer, &in_buffer_size, sl1, PREVIEW_LARGE);
  if (err)
    {
      fprintf (stderr, "convert.c: write_large_preview() : can't read preview file.\n");
      goto cleanup;
    }

  transform_buffer_size = 400 * 300 * 3;
  transform_buffer = xalloc (transform_buffer_size);

  // crop and resize
  int y = 0;
  int x = -1;
  int out_y = 0;
  int out_x = -1;
  for (size_t i = 0; i < in_buffer_size; i+=4)
    {
      x++;
      if (x == 800)
        {
          x = 0;
          y++;
        }

      if (y % 2 > 0)
        continue;

      if (x % 2 > 0)
        continue;

      out_x++;
      if (out_x == 400)
        {
          out_x = 0;
          out_y++;
        }

      transform_buffer[(30+out_y)*1200+out_x*3] = in_buffer[i];
      transform_buffer[(30+out_y)*1200+out_x*3+1] = in_buffer[i+1];
      transform_buffer[(30+out_y)*1200+out_x*3+2] = in_buffer[i+2];
    }

  err = encode_rle15 (&out_buffer, &out_buffer_size, transform_buffer, transform_buffer_size);
  if (err)
    {
      fprintf (stderr, "convert.c: write_large_preview(): can't encode to RLE15.\n");
      goto cleanup;
    }

  count = fwrite (out_buffer, 1, out_buffer_size, file);
  if ((size_t) count != out_buffer_size)
    {
      err = 1;
      fprintf (stderr, "convert.c: write_large_preview(): error while writing to file.\n");
      goto cleanup;
    }

  *index += out_buffer_size;
  headers->image_length = out_buffer_size;
  headers->resolution_x = 400;
  headers->resolution_y = 300;

  fseek (file, ctb->headers.large_preview_offset, SEEK_SET);
  count = fwrite (headers, 1, sizeof (ctb_preview_t), file);
  if ((size_t) count != sizeof (ctb_preview_t))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_large_preview(): error while writing large preview headers.\n");
      goto cleanup;
    }
  fseek (file, *index, SEEK_SET);

  cleanup:
  if (in_buffer) free (in_buffer);
  if (transform_buffer) free (transform_buffer);
  if (out_buffer) free (out_buffer);
  if (headers) free (headers);
  return err;
}

/*
 * Write the small preview in `file`, filling `ctb` by using `sl1`.
 *
 * The preview images in sl1 and ctb do not have the same size nor
 * aspect ratio, so we need to crop and resize them. The small preview
 * is converted from 400x400 to 200x125.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_small_preview (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;
  uint8_t *in_buffer = NULL;
  size_t in_buffer_size = 0;
  uint8_t *transform_buffer = NULL;
  size_t transform_buffer_size = 0;
  uint8_t *out_buffer = NULL;
  size_t out_buffer_size = 0;
  ctb_preview_t *headers = NULL;

  ctb->headers.small_preview_offset = *index;

  headers = xalloc (sizeof (ctb_preview_t));

  int count = fwrite (headers, 1, sizeof (ctb_preview_t), file);
  if ((size_t) count != sizeof (ctb_preview_t))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_small_preview(): error allocating memory for preview headers in file.\n");
      goto cleanup;
    }
  *index += sizeof (ctb_preview_t);
  headers->image_offset = *index;

  err = sl1_read_preview_file (&in_buffer, &in_buffer_size, sl1, PREVIEW_SMALL);
  if (err)
    {
      fprintf (stderr, "convert.c: write_small_preview() : can't read preview file.\n");
      goto cleanup;
    }

  transform_buffer_size = 200 * 125 * 3;
  transform_buffer = xalloc (transform_buffer_size);

  int y = 0;
  int x = -1;
  int out_y = 0;
  int out_x = -1;

  for (size_t i = 0; i < in_buffer_size; i+=4)
    {
      x++;
      if (x == 400)
        {
          x = 0;
          y++;
        }

      if (y % 2 > 0)
        continue;

      if (x % 2 > 0)
        continue;

      if (y < 75)
        continue;

      if (y > 325)
        continue;

      out_x++;
      if (out_x == 400)
        {
          out_x = 0;
          out_y++;
        }

      transform_buffer[out_y*1200+out_x*3] = in_buffer[i];
      transform_buffer[out_y*1200+out_x*3+1] = in_buffer[i+1];
      transform_buffer[out_y*1200+out_x*3+2] = in_buffer[i+2];
    }

  err = encode_rle15 (&out_buffer, &out_buffer_size, transform_buffer, transform_buffer_size);
  if (err)
    {
      fprintf (stderr, "convert.c: write_small_preview(): can't encode to RLE15.\n");
      goto cleanup;
    }

  count = fwrite (out_buffer, 1, out_buffer_size, file);
  if ((size_t) count != out_buffer_size)
    {
      err = 1;
      fprintf (stderr, "convert.c: write_small_preview(): error while writing to file.\n");
      goto cleanup;
    }

  *index += out_buffer_size;
  headers->image_length = out_buffer_size;
  headers->resolution_x = 200;
  headers->resolution_y = 125;

  fseek (file, ctb->headers.small_preview_offset, SEEK_SET);
  count = fwrite (headers, 1, sizeof (ctb_preview_t), file);
  if ((size_t) count != sizeof (ctb_preview_t))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_small_preview(): error while writing small preview headers.\n");
      goto cleanup;
    }
  fseek (file, *index, SEEK_SET);

  cleanup:
  if (in_buffer) free (in_buffer);
  if (transform_buffer) free (transform_buffer);
  if (out_buffer) free (out_buffer);
  if (headers) free (headers);
  return err;
}

/*
 * Write the print config part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_print_config (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;

  ctb->headers.print_config_offset = *index;
  ctb->headers.print_config_size = sizeof (ctb->print_config);

  ctb->print_config.bottom_lift_height = 7;
  ctb->print_config.bottom_lift_speed = 70;
  ctb->print_config.lift_height = 7;
  ctb->print_config.lift_speed = 70;
  ctb->print_config.retract_speed = 210;
  ctb->print_config.resin_volume = sl1->bottle_volume;
  ctb->print_config.resin_mass = 3;
  ctb->print_config.resin_cost = sl1->bottle_cost;
  ctb->print_config.bottom_light_off_delay = 0;
  ctb->print_config.light_off_delay = 0;
  ctb->print_config.bottom_layer_count = sl1->faded_layers;

  int count = fwrite (&ctb->print_config, 1, sizeof (ctb->print_config), file);
  if ((size_t) count != sizeof (ctb->print_config))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_print_config(): error while writing to file.\n");
      goto cleanup;
    }

  (*index) += sizeof (ctb->print_config);

  cleanup:
  return err;
}

/*
 * Write the slicer config part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_slicer_config (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;

  ctb->headers.slicer_config_size = sizeof (ctb->slicer_config);

  ctb->slicer_config.bottom_lift_height2 = 0;
  ctb->slicer_config.bottom_lift_speed2 = 0;
  ctb->slicer_config.lift_height2 = 0;
  ctb->slicer_config.lift_speed2 = 0;
  ctb->slicer_config.retract_distance2 = 0;
  ctb->slicer_config.retract_speed2 = 0;
  ctb->slicer_config.reset_time_after_lift = 0;
  ctb->slicer_config.per_layer_settings = v3 ? 0x2000000F : 0x4000000F;
  ctb->slicer_config.mysterious_id = 27345357;
  ctb->slicer_config.antialias_level = 4;
  ctb->slicer_config.software_version = 17367040;
  ctb->slicer_config.rest_time_after_retract = 0.5;
  ctb->slicer_config.rest_time_after_lift2 = 0;
  ctb->slicer_config.transition_layer_count = 10;

  int count = fwrite (&ctb->slicer_config, 1, sizeof (ctb->slicer_config), file);
  if ((size_t) count != sizeof (ctb->slicer_config))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_slicer_config(): error while writing to file.\n");
      goto cleanup;
    }

  (*index) += sizeof (ctb->slicer_config);

  cleanup:
  return err;
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_machine_name (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;
  size_t len = strlen (sl1->printer_settings_id);
  ctb->slicer_config.machine_type_offset = *index;
  ctb->slicer_config.machine_type_len = len;

  int count = fwrite (sl1->printer_settings_id, 1, len, file);
  if ((size_t) count != len)
    {
      err = 1;
      fprintf (stderr, "convert.c: write_machine_name(): error while writing to file.\n");
      goto cleanup;
    }

  (*index) += len;

  cleanup:
  return err;
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_v4_disclaimer (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;
  const char *disclaimer = "Layout and record format for the ctb and cbddlp file types are the copyrighted programs or codes of CBD Technology (China) Inc..The Customer or User shall not in any manner reproduce, distribute, modify, decompile, disassemble, decrypt, extract, reverse engineer, lease, assign, or sublicense the said programs or codes.";
  size_t len = strlen (disclaimer);
  ctb->print_config_v4.disclaimer_offset = *index;
  ctb->print_config_v4.disclaimer_len = len;

  int count = fwrite (disclaimer, 1, len, file);
  if ((size_t) count != len)
    {
      err = 1;
      fprintf (stderr, "convert.c: write_v4_disclaimer(): error while writing to file.\n");
      goto cleanup;
    }

  (*index) += len;

  cleanup:
  return err;
}

/*
 * Write the print_config_v4 part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_v4_config (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;
  ctb->slicer_config.print_config_v4_offset = *index;

  ctb->print_config_v4.bottom_retract_speed = 210;
  ctb->print_config_v4.bottom_retract_speed2 = 0;
  ctb->print_config_v4.four1 = 4;
  ctb->print_config_v4.four2 = 4;
  ctb->print_config_v4.rest_time_after_retract = 0.5;
  ctb->print_config_v4.rest_time_after_lift = 0;
  ctb->print_config_v4.rest_time_before_lift = 0;
  ctb->print_config_v4.bottom_retract_height2 = 0;
  ctb->print_config_v4.unknown1 = 29008;
  ctb->print_config_v4.unknown2 = 267967;
  ctb->print_config_v4.unknown3 = 4;
  ctb->print_config_v4.last_layer_index = sl1->num_fast_layers + sl1->num_slow_layers - 1;

  int count = fwrite (&ctb->print_config_v4, 1, sizeof (ctb->print_config_v4), file);
  if ((size_t) count != sizeof (ctb->print_config_v4))
    {
      err = 1;
      fprintf (stderr, "convert.c: write_v4_config(): error while writing to file.\n");
      goto cleanup;
    }

  (*index) += sizeof (ctb->print_config_v4);

  cleanup:
  return err;
}

void
rle1_add_pixel (uint8_t *out_buffer, uint8_t color, uint32_t stride, size_t *out_buffer_size)
{
  if (stride == 0)
    return;

  if (stride > 1)
    color |= 0x80;

  out_buffer[(*out_buffer_size)++] = color;

  if (stride <= 1)
    return;

  if (stride <= 0x7f)
    {
      out_buffer[(*out_buffer_size)++] = (uint8_t) stride;
      return;
    }

  if (stride <= 0x3fff)
    {
      out_buffer[(*out_buffer_size)++] = (uint8_t) ((stride >> 8) | 0x80);
      out_buffer[(*out_buffer_size)++] = (uint8_t) stride;
      return;
    }

  if (stride <= 0x1fffff)
    {
      out_buffer[(*out_buffer_size)++] = (uint8_t) ((stride >> 16) | 0xc0);
      out_buffer[(*out_buffer_size)++] = (uint8_t) (stride >> 8);
      out_buffer[(*out_buffer_size)++] = (uint8_t) stride;
      return;
    }

  if (stride <= 0xfffffff)
    {
      out_buffer[(*out_buffer_size)++] = (uint8_t) ((stride >> 24) | 0xe0);
      out_buffer[(*out_buffer_size)++] = (uint8_t) (stride >> 16);
      out_buffer[(*out_buffer_size)++] = (uint8_t) (stride >> 8);
      out_buffer[(*out_buffer_size)++] = (uint8_t) stride;
      return;
    }
}

static int
encode_rle1 (uint8_t **out_buffer, size_t *out_buffer_size, const uint8_t *in_buffer, size_t in_buffer_size)
{
  int err = 0;
  uint8_t color = UINT8_MAX >> 1;
  uint32_t stride = 0;
  uint8_t *tmp_buffer = xalloc (in_buffer_size);

  for (size_t i = 0; i < in_buffer_size; i++)
    {
      uint8_t grey7 = in_buffer[i] >> 1;
      if (grey7 == color)
        stride++;
      else
        {
          rle1_add_pixel (tmp_buffer, color, stride, out_buffer_size);
          color = grey7;
          stride = 1;
        }
    }

  rle1_add_pixel (tmp_buffer, color, stride, out_buffer_size);

  *out_buffer = xalloc (*out_buffer_size);
  memcpy (*out_buffer, tmp_buffer, *out_buffer_size);

  if (tmp_buffer) free (tmp_buffer);
  return err;
}

static int
encrypt_layer (uint8_t **out_buffer, size_t *out_buffer_size, const uint8_t *encoded_buffer, size_t encoded_buffer_size, uint32_t seed, uint32_t layer_index)
{
  int err = 0;

  if (seed == 0)
    {
      err = 1;
      fprintf (stderr, "convert.c: encrypt_layer(): encryption key should not be 0.\n");
      goto cleanup;
    }

  uint32_t init = seed * 0x2d83cdac + 0xd8a83423;
  uint32_t key = (layer_index * 0x1e1530cd + 0xec3d47cd) * init;
  *out_buffer_size = encoded_buffer_size;
  *out_buffer = xalloc (*out_buffer_size);

  int index = 0;
  for (size_t i = 0; i < encoded_buffer_size; i++)
    {
      uint8_t k = (uint8_t) (key >> (8 * index));
      index++;

      if ((index & 3) == 0)
        {
          key += init;
          index = 0;
        }

      (*out_buffer)[i] = (uint8_t) (encoded_buffer[i] ^ k);
    }

  cleanup:
  return err;
}

/*
 * Write the headers part of the ctb file in `file`, filling `ctb` by using `sl1`.
 *
 * The current index of data in file after writing is updated in `index`.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
static int
write_layers (FILE *file, size_t *index, ctb_t *ctb, const sl1_t *sl1, bool v3)
{
  int err = 0;
  double z = 0;
  size_t layers_count = (size_t) sl1->num_fast_layers + (size_t) sl1->num_slow_layers;
  uint8_t *in_buffer = NULL;
  size_t in_buffer_size = 0;
  uint8_t *encoded_buffer = NULL;
  size_t encoded_buffer_size = 0;
  uint8_t *out_buffer = NULL;
  size_t out_buffer_size = 0;

  ctb->headers.layer_table_offset = *index;
  //ctb->headers.encryption_key = (uint32_t) rand();
  ctb->headers.encryption_key = 207222928;

  ctb_layer_header_base_t headers[layers_count];
  for (uint32_t i = 0; i < layers_count; i++)
    {
      ctb_layer_header_base_t header = {0};
      headers[i] = header;
    }

  int count = fwrite (headers, 1, sizeof (ctb_layer_header_base_t) * layers_count, file);
  if ((size_t) count != sizeof (ctb_layer_header_base_t) * layers_count)
    {
      err = 1;
      fprintf (stderr, "convert.c: write_layers(): error while preallocating headers in file.\n");
      goto cleanup;
    }
  (*index) += sizeof (ctb_layer_header_base_t) * layers_count;

  // write layers
  for (size_t i = 0; i < layers_count; i++)
    {
      int ext_pos = *index;
      ctb_layer_header_extended_t ext = {0};

      count = fwrite (&ext, 1, sizeof (ctb_layer_header_extended_t), file);
      if ((size_t) count != sizeof (ctb_layer_header_extended_t))
        {
          err = 1;
          fprintf (stderr, "convert.c: write_layers(): error while preallocating extended headers in file.\n");
          goto cleanup;
        }
      (*index) += sizeof (ctb_layer_header_extended_t);

      headers[i].data_offset = *index;

      err = sl1_read_layer_image_file (&in_buffer, &in_buffer_size, sl1, i);
      if (err)
        {
          fprintf (stderr, "convert.c: write_layers() : can't read layer image file %ld.\n", i);
          goto cleanup;
        }

      err = encode_rle1 (&encoded_buffer, &encoded_buffer_size, in_buffer, in_buffer_size);
      if (err)
        {
          fprintf (stderr, "convert.c: write_layers(): can't encode layer %ld to RLE1.\n", i);
          goto cleanup;
        }

      err = encrypt_layer (&out_buffer, &out_buffer_size, encoded_buffer, encoded_buffer_size, ctb->headers.encryption_key, i);
      if (err)
        {
          fprintf (stderr, "convert.c: write_layers(): can't encode layer %ld to RLE1.\n", i);
          goto cleanup;
        }

      count = fwrite (out_buffer, 1, out_buffer_size, file);
      if ((size_t) count != out_buffer_size)
        {
          err = 1;
          fprintf (stderr, "convert.c: write_layers(): error while writing layer %ld to file.\n", i);
          goto cleanup;
        }

      headers[i].data_len = out_buffer_size;
      (*index) += out_buffer_size;

      z += sl1->layer_height;

      ext.z = z;
      ext.data_offset = headers[i].data_offset;
      ext.data_len = headers[i].data_len;
      ext.table_size = 84;
      ext.total_size = out_buffer_size + sizeof (ctb_layer_header_base_t);
      ext.lift_height = 7;
      ext.lift_speed = 70;
      ext.retract_speed = 210;
      ext.rest_time_after_retract = 0.5;
      ext.light_pwm = 255;

      headers[i].z = z;
      headers[i].table_size = 84;

      if (i < (size_t) sl1->faded_layers)
        {
          ext.exposure_time = sl1->initial_exposure_time;
          headers[i].exposure_time = sl1->initial_exposure_time;
        }
      else
        {
          if (i <= (size_t) sl1->faded_layers + 10)
            {
              ext.exposure_time = sl1->initial_exposure_time - ((i+1 - sl1->faded_layers) * sl1->exposure_time);
              headers[i].exposure_time = headers[i].exposure_time;
              ext.lift_height = headers[i].exposure_time;
            }
          else
            {
              ext.exposure_time = sl1->exposure_time;
              headers[i].exposure_time = sl1->exposure_time;
              ext.lift_height = sl1->exposure_time;
            }
        }

      fseek (file, ext_pos, SEEK_SET);
      count = fwrite (&ext, 1, sizeof (ctb_layer_header_extended_t), file);
      if ((size_t) count != sizeof (ctb_layer_header_extended_t))
        {
          err = 1;
          fprintf (stderr, "convert.c: write_layers(): error while writing extended headers to file.\n");
          goto cleanup;
        }
      fseek (file, *index, SEEK_SET);

      if (in_buffer) free (in_buffer);
      if (encoded_buffer) free (encoded_buffer);
      if (out_buffer) free (out_buffer);
      in_buffer = NULL;
      in_buffer_size = 0;
      encoded_buffer = NULL;
      encoded_buffer_size = 0;
      out_buffer = NULL;
      out_buffer_size = 0;
    }

  fseek (file, ctb->headers.layer_table_offset, SEEK_SET);
  for (size_t i = 0; i < layers_count; i++)
    {

      ctb_layer_header_base_t header = headers[i];

      int count = fwrite (&header, 1, sizeof (ctb_layer_header_base_t), file);
      if ((size_t) count != sizeof (ctb_layer_header_base_t))
        {
          err = 1;
          fprintf (stderr, "convert.c: write_layers(): error while writing headers to file.\n");
          goto cleanup;
        }

      (*index) += sizeof (ctb_layer_header_base_t);
    }
  fseek (file, *index, SEEK_SET);

  cleanup:
  if (in_buffer) free (in_buffer);
  if (encoded_buffer) free (encoded_buffer);
  if (out_buffer) free (out_buffer);
  return err;
}

// PUBLIC

/*
 * Convert the file at `in` into a ctb file at `out`.
 *
 * It will be a ctb version 4 file, unless the `v3` flag
 * is set.
 *
 * Returns non-zero on error.
 */
int
convert (const char *in, const char *out, const bool v3)
{
  int ret = 0;
  sl1_t *sl1 = NULL;

  if (!is_sl1_file (in))
    {
      fprintf (stderr, "convert.c: Input file is not a sl1 file.\n");
      ret = 1;
      goto cleanup;
    }

  sl1 = new_sl1 ();
  if (parse_sl1_archive (sl1, in))
    {
      fprintf (stderr, "convert.c: Can't parse input file.\n");
      ret = 1;
      goto cleanup;
    }

  if (write_file (sl1, out, v3))
    {
      fprintf (stderr, "convert.c: Can't write output file.\n");
      ret = 1;
      goto cleanup;
    }

  cleanup:
  if (sl1) free_sl1 (sl1);
  return ret;
}
