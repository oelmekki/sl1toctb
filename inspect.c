#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "utils.h"
#include "parser.h"
#include "spng.h"

static void   display_sl1     (sl1_t *sl1);
static void   display_ctb     (ctb_t *ctb);
static int    export_layer    (ctb_t *ctb, FILE *ctb_file, FILE *metadata_file, const char *dir, int i);

// TODO
/*
 * Display meta data about given sl1 file.
 */
static void
display_sl1 (sl1_t *sl1)
{
}

/*
 * Display meta data about given ctb file.
 */
static void
display_ctb (ctb_t *ctb)
{
  printf ("CTB file, version %d for %s\n", ctb->headers.version, ctb->machine_name);

  puts ("\n[Header]");
  printf ("Bed size (x, y, z): %.0fx%.0fx%.0fmm\n", ctb->headers.bed_size_x, ctb->headers.bed_size_y, ctb->headers.bed_size_z);
  printf ("Height of the printed object: %.2fmm.\n", ctb->headers.total_height);
  printf ("Indicative layer height: %.2fmm\n", ctb->headers.layer_height);
  printf ("Indicative exposure: %.2fs\n", ctb->headers.layer_exposure);
  printf ("Indicative exposure at bottom: %.2fs\n", ctb->headers.bottom_exposure);
  printf ("Indicative light off delay: %.2fs\n", ctb->headers.light_off_delay);
  printf ("Bottom layer count: %d\n", ctb->headers.bottom_layer_count);
  printf ("Resolution (x, y): %dx%d\n", ctb->headers.resolution_x, ctb->headers.resolution_y);
  printf ("Layer count: %d\n", ctb->headers.layer_count);
  printf ("Estimated print time: %ds.\n", ctb->headers.print_time);
  if (ctb->headers.projection)
    puts ("Projection is mirrored.");
  printf ("Antialias level: %d\n", ctb->headers.antialias_level);
  printf ("PWM level: %d\n", ctb->headers.pwm_level);
  printf ("Bottom PWM level: %d\n", ctb->headers.bottom_pwm_level);
  if (ctb->headers.encryption_key)
    printf ("File is encrypted! Key: %d\n", ctb->headers.encryption_key);
  printf ("Layer table offset: %d\n", ctb->headers.layer_table_offset);
  printf ("Large preview offset: %d\n", ctb->headers.large_preview_offset);
  printf ("Small preview offset: %d\n", ctb->headers.small_preview_offset);
  printf ("Print config offset: %d\n", ctb->headers.print_config_offset);
  printf ("Slicer config offset: %d\n", ctb->headers.slicer_config_offset);

  puts ("\n[Print config]");
  printf ("Bottom lift height: %.2fmm.\n", ctb->print_config.bottom_lift_height);
  printf ("Bottom lift speed: %.2fmm/min.\n", ctb->print_config.bottom_lift_speed);
  printf ("Lift height: %.2fmm.\n", ctb->print_config.lift_height);
  printf ("Lift speed: %.2fmm/min.\n", ctb->print_config.lift_speed);
  printf ("Retract speed: %.2fmm/min.\n", ctb->print_config.retract_speed);
  printf ("Resin needed: %.2fml, %.2fg.\n", ctb->print_config.resin_volume, ctb->print_config.resin_mass);
  printf ("Resin cost: %.2f€.\n", ctb->print_config.resin_cost);

  puts ("\n[Slicer config]");
  printf ("Bottom lift height 2: %.2fmm.\n", ctb->slicer_config.bottom_lift_height2);
  printf ("Bottom lift speed 2: %.2fmm/sec.\n", ctb->slicer_config.bottom_lift_speed2);
  printf ("Lift height 2: %.2fmm.\n", ctb->slicer_config.lift_height2);
  printf ("Lift distance speed 2: %.2fmm/sec.\n", ctb->slicer_config.lift_speed2);
  printf ("Retract distance 2: %.2fmm.\n", ctb->slicer_config.retract_distance2);
  printf ("Retract speed 2: %.2fmm/sec.\n", ctb->slicer_config.retract_speed2);
  printf ("Reset time after lift: %.2fsec.\n", ctb->slicer_config.reset_time_after_lift);

  switch (ctb->slicer_config.per_layer_settings)
    {
      case 0xF:
        puts ("Per layer settings are disabled.");
        break;

      case 0x2000000F:
        puts ("Per layer settings are enabled (v3).");
        break;

      case 0x4000000F:
        puts ("Per layer settings are enabled (v4).");
        break;
    }

  printf ("Mysterious id is %d.\n", ctb->slicer_config.mysterious_id);
  printf ("Antialias level: %d\n", ctb->slicer_config.antialias_level);
  printf ("Software version: %X\n", ctb->slicer_config.software_version);
  printf ("Rest time after retract: %.2f\n", ctb->slicer_config.rest_time_after_retract);
  printf ("Rest time after lift 2: %.2f\n", ctb->slicer_config.rest_time_after_lift2);
  printf ("Transition layer count : %d\n", ctb->slicer_config.transition_layer_count);
  printf ("Machine type offset: %d\n", ctb->slicer_config.machine_type_offset);
  printf ("Print config v4 offset: %d\n", ctb->slicer_config.print_config_v4_offset);

  if (ctb->headers.version == 4)
    {
      puts ("\n[Print config v4]");
      printf ("Bottom retract speed: %.2f\n", ctb->print_config_v4.bottom_retract_speed);
      printf ("Bottom retract speed 2: %.2f\n", ctb->print_config_v4.bottom_retract_speed2);
      printf ("Rest time after retract : %.2f\n", ctb->print_config_v4.rest_time_after_retract);
      printf ("Rest time after lift : %.2f\n", ctb->print_config_v4.rest_time_after_lift);
      printf ("Rest time before lift : %.2f\n", ctb->print_config_v4.rest_time_before_lift);
      printf ("Bottom retract height 2 : %.2f\n", ctb->print_config_v4.bottom_retract_height2);
      printf ("Disclaimer offset : %d\n", ctb->print_config_v4.disclaimer_offset);
      printf ("Disclaimer : %s\n", ctb->v4_disclaimer);
    }

  puts ("\n[Large preview]");
  printf ("Resolution: %dx%d\n", ctb->large_preview.resolution_x, ctb->large_preview.resolution_y);
  printf ("Image offset : %d\n", ctb->large_preview.image_offset);

  puts ("\n[Small preview]");
  printf ("Resolution: %dx%d\n", ctb->small_preview.resolution_x, ctb->small_preview.resolution_y);
  printf ("Image offset : %d\n", ctb->small_preview.image_offset);
}

/*
 * Extract next image in `file` to the directory `dir`,
 * naming it with `i`.
 *
 * `file` must be already seek to the proper entry in the
 * layer headers table.
 *
 * Returns non-zero in case of error.
 */
static int
export_layer (ctb_t *ctb, FILE *ctb_file, FILE *metadata_file, const char *dir, int layer_index)
{
  int err = 0;
  uint8_t *raw_data = NULL;
  uint8_t *data = NULL;
  spng_ctx *ctx = NULL;
  FILE *file = NULL;

  ctb_layer_header_t *hdr = &ctb->layer_headers[layer_index];

  err = fseek (ctb_file, hdr->base.data_offset, SEEK_SET);
  if (err)
    {
      fprintf (stderr, "inspect.c: export_layer() : can't seek ctb file.\n");
      goto cleanup;
    }

  raw_data = xalloc (hdr->base.data_len);
  size_t count = fread (raw_data, hdr->base.data_len, 1, ctb_file);
  if (count != 1)
    {
      err = 1;
      fprintf (stderr, "inspect.c: export_layer() : can't read layer raw data.\n");
      goto cleanup;
    }

  if (ctb->headers.encryption_key)
    decrypt_layer (raw_data, ctb->headers.encryption_key, layer_index, hdr->base.data_len);

  size_t data_len = 0;
  size_t nonzero_pixels_count = 0;
  err = decode_layer (&data, &data_len, raw_data, hdr->base.data_len, ctb, &nonzero_pixels_count);
  if (err)
    {
      fprintf (stderr, "inspect.c: export_layer() : can't decode layer data.\n");
      goto cleanup;
    }

  fprintf (metadata_file, "[Layer %d]\n", layer_index);
  fprintf (metadata_file, "Non-zero pixels: %ld\n", nonzero_pixels_count);

  fprintf (metadata_file, "BASE\n");
  fprintf (metadata_file, "Z: %.3f\n", hdr->base.z);
  fprintf (metadata_file, "Exposure: %.3f secs\n", hdr->base.exposure_time);
  fprintf (metadata_file, "Light Off Time: %.3f secs\n", hdr->base.light_off_delay);
  fprintf (metadata_file, "Data offset: %u\n", hdr->base.data_offset);
  fprintf (metadata_file, "Data len: %u\n", hdr->base.data_len);
  fprintf (metadata_file, "Unknown1: %u\n", hdr->base.unknown1);
  fprintf (metadata_file, "Table size: %u\n", hdr->base.table_size);
  fprintf (metadata_file, "Unknown2: %u\n", hdr->base.unknown2);
  fprintf (metadata_file, "Unknown3: %u\n", hdr->base.unknown3);

  fprintf (metadata_file, "EXTENDED\n");
  fprintf (metadata_file, "Z: %.3f\n", hdr->extended.z);
  fprintf (metadata_file, "Exposure: %.3f secs\n", hdr->extended.exposure_time);
  fprintf (metadata_file, "Light Off Time: %.3f secs\n", hdr->extended.light_off_delay);
  fprintf (metadata_file, "Data offset: %u\n", hdr->extended.data_offset);
  fprintf (metadata_file, "Data len: %u\n", hdr->extended.data_len);
  fprintf (metadata_file, "Unknown1: %u\n", hdr->extended.unknown1);
  fprintf (metadata_file, "Table size: %u\n", hdr->extended.table_size);
  fprintf (metadata_file, "Unknown2: %u\n", hdr->extended.unknown2);
  fprintf (metadata_file, "Unknown3: %u\n", hdr->extended.unknown3);
  fprintf (metadata_file, "Lift height: %.3f mm\n", hdr->extended.lift_height);
  fprintf (metadata_file, "Lift speed: %.3f mm/s\n", hdr->extended.lift_speed);
  fprintf (metadata_file, "Lift height2: %.3f mm\n", hdr->extended.lift_height2);
  fprintf (metadata_file, "Lift speed2: %.3f mm/s\n", hdr->extended.lift_speed2);
  fprintf (metadata_file, "Retract speed: %.3f mm/s\n", hdr->extended.retract_speed);
  fprintf (metadata_file, "Retract distance2: %.3f mm\n", hdr->extended.retract_height2);
  fprintf (metadata_file, "Retract speed2: %.3f mm/s\n", hdr->extended.retract_speed2);
  fprintf (metadata_file, "Rest time before lift: %.3f secs\n", hdr->extended.rest_time_before_lift);
  fprintf (metadata_file, "Rest time after lift: %.3f secs\n", hdr->extended.rest_time_after_lift);
  fprintf (metadata_file, "Rest time after retract: %.3f secs\n", hdr->extended.rest_time_after_retract);
  fprintf (metadata_file, "Light PWM: %.2f secs\n", hdr->extended.light_pwm);
  fputs ("\n", metadata_file);

  char filename[250] = "";
  snprintf (filename, 249, "%s/%04d.png", dir, layer_index);
  file = fopen (filename, "wb");
  if (!file)
    {
      err = 1;
      fprintf (stderr, "inspect.c: can't open file for writing: %s\n", filename);
      goto cleanup;
    }

  ctx = spng_ctx_new (SPNG_CTX_ENCODER);

  err = spng_set_png_file (ctx, file);
  if (err)
    {
      fprintf (stderr, "inspect.c: export_layer() : Error while passing file to spng.\n");
      err = 1;
      goto cleanup;
    }

  struct spng_ihdr headers = {
    .width = ctb->headers.resolution_x,
    .height = ctb->headers.resolution_y,
    .bit_depth = 8,
    .color_type = SPNG_COLOR_TYPE_GRAYSCALE,
    .compression_method = 0,
    .filter_method = SPNG_FILTER_NONE,
    .interlace_method = SPNG_INTERLACE_NONE,
  };

  err = spng_set_ihdr (ctx, &headers);
  if (err)
    {
      fprintf (stderr, "inspect.c: export_layer() : Can't set headers.\n");
      goto cleanup;
    }

  err = spng_encode_image (ctx, data, data_len, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
  if (err)
    {
      fprintf (stderr, "inspect.c: export_layer() : Can't encode image: %s.\n", spng_strerror (err));
      goto cleanup;
    }

  cleanup:
  if (raw_data) free (raw_data);
  if (data) free (data);
  if (file) fclose (file);
  if (ctx) spng_ctx_free (ctx);
  return err;
}

// PUBLIC

/*
 * Extract large preview image and display it.
 *
 * `image_type` is PREVIEW_LARGE or PREVIEW_SMALL.
 *
 * Returns non-zero in case of error.
 */
int
show_preview_image (const char *filename, int image_type)
{
  int err = 0;
  uint8_t *data = NULL;
  FILE *file = NULL;
  size_t len = 0;
  spng_ctx *ctx = NULL;
  char tmp_file[] = "/tmp/sl1toctb-XXXXXX";
  mkstemp (tmp_file);
  bool file_created = false;

  ctb_t *ctb = new_ctb ();
  err = parse_ctb_file (ctb, filename);
  if (err)
    {
      fprintf (stderr, "inspect.c: show_preview_image() : Unrecognized file at %s\n", filename);
      goto cleanup;
    }

  err = ctb_read_preview_file (&data, &len, ctb, image_type);
  if (err)
    {
      fprintf (stderr, "inspect.c: show_preview_image() : Can't read image data\n");
      goto cleanup;
    }

  file = fopen (tmp_file, "wb");
  if (!file)
    {
      fprintf (stderr, "inspect.c: show_preview_image() : Can't open destination file.\n");
      err = 1;
      goto cleanup;
    }

  file_created = true;
  ctb_preview_t *preview = image_type == PREVIEW_LARGE ? &ctb->large_preview : &ctb->small_preview;

  uint32_t width = preview->resolution_x;
  uint32_t height = preview->resolution_y;

  ctx = spng_ctx_new (SPNG_CTX_ENCODER);
  err = spng_set_png_file (ctx, file);
  if (err)
    {
      fprintf (stderr, "inspect.c: show_preview_image() : Error while passing file to spng.\n");
      err = 1;
      goto cleanup;
    }

  struct spng_ihdr headers = {
    .width = width,
    .height = height,
    .bit_depth = 8,
    .color_type = SPNG_COLOR_TYPE_TRUECOLOR,
    .compression_method = 0,
    .filter_method = SPNG_FILTER_NONE,
    .interlace_method = SPNG_INTERLACE_NONE,
  };

  err = spng_set_ihdr (ctx, &headers);
  if (err)
    {
      fprintf (stderr, "inspect.c: show_preview_image() : Can't set headers.\n");
      goto cleanup;
    }


  err = spng_encode_image (ctx, data, len, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
  if (err)
    {
      fprintf (stderr, "inspect.c: show_preview_image() : Can't encode image: %s.\n", spng_strerror (err));
      goto cleanup;
    }

  fclose (file);
  file = NULL;

  char cmd[250] = "";
  snprintf (cmd, 249, "sxiv %s", tmp_file);
  system (cmd);

  cleanup:
  if (ctb) free_ctb (ctb);
  if (data) free (data);
  if (file) fclose (file);
  if (ctx) spng_ctx_free (ctx);
  if (file_created) unlink (tmp_file);

  return err;
}

/*
 * Inspect given file, showing its type and most relevant
 * information.
 */
int
inspect (const char *in)
{
  int ret;
  sl1_t *sl1 = NULL;
  ctb_t *ctb = NULL;

  sl1 = new_sl1 ();
  int err = parse_sl1_archive (sl1, in);
  if (!err)
    {
      display_sl1 (sl1);
      ret = 0;
      goto cleanup;
    }

  ctb = new_ctb ();
  err = parse_ctb_file (ctb, in);
  if (err)
    {
      fprintf (stderr, "inspect() : Unrecognized file at %s\n", in);
      ret = 1;
      goto cleanup;
    }
  else
    {
      display_ctb (ctb);
      ret = 0;
      goto cleanup;
    }

  cleanup:
  if (sl1) free_sl1 (sl1);
  if (ctb) free_ctb (ctb);
  return ret;
}

/*
 * Export each layer image from ctb file at `filename` to the directory at
 * `dir`.
 *
 * Returns non-zero in case of error.
 */
int
export_layers (const char *filename, const char *dir)
{
  int err = 0;
  ctb_t *ctb = NULL;
  FILE *ctb_file = NULL;
  FILE *metadata_file = NULL;
  char metadata_filename[750] = "";
  struct stat s;

  err = stat (dir, &s);
  if (err)
    {
      fprintf (stderr, "export_layers() : directory %s does not exists.\n", dir);
      goto cleanup;
    }

  if (!S_ISDIR (s.st_mode))
    {
      fprintf (stderr, "export_layers() : %s is not a directory.\n", dir);
      err = 1;
      goto cleanup;
    }

  snprintf (metadata_filename, 749, "%s/metadata.txt", dir);

  metadata_file = fopen (metadata_filename, "w");
  if (!metadata_file)
    {
      fprintf (stderr, "export_layers() : can't open file %s for writing.\n", metadata_filename);
      err = 1;
      goto cleanup;
    }

  ctb_file = fopen (filename, "rb");
  if (!ctb_file)
    {
      fprintf (stderr, "export_layers() : can't read file %s.\n", filename);
      err = 1;
      goto cleanup;
    }

  ctb = new_ctb ();
  err = parse_ctb_file (ctb, filename);
  if (err)
    {
      fprintf (stderr, "export_layers() : Can't parse ctb file.\n");
      goto cleanup;
    }

  fseek (ctb_file, ctb->headers.layer_table_offset, SEEK_SET);

  for (size_t i = 0; i < ctb->headers.layer_count; i++)
    {
      err = export_layer (ctb, ctb_file, metadata_file, dir, i);
      if (err)
        {
          fprintf (stderr, "export_layers() : error while processing layer %ld.\n", i);
          goto cleanup;
        }
    }

  cleanup:
  if (ctb) free_ctb (ctb);
  if (ctb_file) fclose (ctb_file);
  if (metadata_file) fclose (metadata_file);
  return err;
}
