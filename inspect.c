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
  printf ("CTB file, version %d for %s\n", ctb->headers.version, ctb->machine_name);

  puts ("\n[Header]");
  printf ("Bed size (x, y, z): %.0fx%.0fx%.0fmm\n", ctb->headers.bed_size_x, ctb->headers.bed_size_y, ctb->headers.bed_size_z);
  printf ("Height of the printed object: %.2fmm.\n", ctb->headers.total_height);
  printf ("Indicative layer height: %.2fmm\n", ctb->headers.layer_height);
  printf ("Indicative exposure: %.2fs\n", ctb->headers.exposure);
  printf ("Indicative exposure at bottom: %.2fs\n", ctb->headers.exposure_bottom);
  printf ("Indicative light off time: %.2fs\n", ctb->headers.light_off_time);
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

  puts ("\n[Print config]");
  printf ("Bottom lift distance: %.2fmm.\n", ctb->print_config.bottom_lift_distance);
  printf ("Bottom lift speed: %.2fmm/min.\n", ctb->print_config.bottom_lift_speed);
  printf ("Lift distance: %.2fmm.\n", ctb->print_config.lift_distance);
  printf ("Lift speed: %.2fmm/min.\n", ctb->print_config.lift_speed);
  printf ("Retract speed: %.2fmm/min.\n", ctb->print_config.retract_speed);
  printf ("Resin needed: %.2fml, %.2fg.\n", ctb->print_config.resin_volume, ctb->print_config.resin_mass);
  printf ("Resin cost: %.2f€.\n", ctb->print_config.resin_cost);

  puts ("\n[Slicer config]");
  printf ("Bottom lift distance 2: %.2fmm.\n", ctb->slicer_config.bottom_lift_distance2);
  printf ("Bottom lift distance speed 2: %.2fmm/sec.\n", ctb->slicer_config.bottom_lift_speed2);
  printf ("Lift distance 2: %.2fmm.\n", ctb->slicer_config.lift_distance2);
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
  printf ("Software version: %X\n", ctb->slicer_config.software_version);
  printf ("Rest time after retract: %.2f\n", ctb->slicer_config.rest_time_after_retract);
  printf ("Rest time after lift 2: %.2f\n", ctb->slicer_config.rest_time_after_lift2);
  printf ("Transition layer count : %d\n", ctb->slicer_config.transition_layer_count);

  if (ctb->headers.version == 4)
    {
      puts ("\n[Print config v4]");
      printf ("Bottom retract speed: %.2f\n", ctb->print_config_v4.bottom_retract_speed);
      printf ("Bottom retract speed 2: %.2f\n", ctb->print_config_v4.bottom_retract_speed2);
      printf ("Rest time after retract : %.2f\n", ctb->print_config_v4.rest_time_after_retract);
      printf ("Rest time after lift : %.2f\n", ctb->print_config_v4.rest_time_after_lift);
      printf ("Rest time before lift : %.2f\n", ctb->print_config_v4.rest_time_before_lift);
      printf ("Bottom retract height 2 : %.2f\n", ctb->print_config_v4.bottom_retract_height2);
      printf ("Disclaimer : %s\n", ctb->disclaimer);
    }

  puts ("\n[Large preview]");
  printf ("Resolution: %dx%d\n", ctb->large_preview.resolution_x, ctb->large_preview.resolution_y);

  puts ("\n[Small preview]");
  printf ("Resolution: %dx%d\n", ctb->small_preview.resolution_x, ctb->small_preview.resolution_y);
}

// PUBLIC

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
  int err = parse_sl1_file (sl1, in);
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
      fprintf (stderr, "Unrecognized file at %s\n", in);
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
