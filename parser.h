#ifndef _PARSER_H
#define _PARSER_H

#include <sys/types.h>

enum {
  CTB_PREVIEW_LARGE,
  CTB_PREVIEW_SMALL,
};

typedef struct {
  int whatever; // FIXME
} sl1_t;

typedef struct {
  u_int32_t magic;                  // 0x12fd0086 for ctb files.
  u_int32_t version;                // ctb file version. We support 3 and 4.
  float bed_size_x;                 // length of the build plate, in mm.
  float bed_size_y;                 // width of the build plate, in mm.
  float bed_size_z;                 // height of the build plate, in mm.
  u_int32_t unknown1;
  u_int32_t unknown2;
  float total_height;               // height of the printed object, in mm.
  float layer_height;               // height of each layer, in mm. Overriden in layer information.
  float exposure;                   // exposure time for normal layers, in secs. Overriden in layer information.
  float exposure_bottom;            // exposure time for bottom layers, in secs. Overriden in layer information.
  float light_off_time;             // light off time setting used at slicing, in secs. Overriden in layer information.
                                    // Also in ext_config.
  u_int32_t bottom_layer_count;     // Number of layer being considered bottom. Also in ext_config.
  u_int32_t resolution_x;           // Printer resolution on x axis, in px.
  u_int32_t resolution_y;           // Printer resolution on y axis, in px.
  u_int32_t large_preview_offset;   // offset in file for large preview image headers.
  u_int32_t layer_table_offset;     // offset in file for layer headers.
  u_int32_t layer_count;            // number of layers in the print.
  u_int32_t small_preview_offset;   // offset in file for small preview image headers.
  u_int32_t print_time;             // estimated print time, in secs.
  u_int32_t projection;             // 0 (normal) or 1 (mirrored)
  u_int32_t print_config_offset;    // offset in file for extended configuration.
  u_int32_t print_config_size;      // size of the extended configuration, in bytes.
  u_int32_t antialias_level;        // unused (legacy from cbddlp). Set to 1.
  u_int16_t pwm_level;              // PWM duty cycle for the UV illumination source on normal level.
                                    // This appears to be an 8-bit quantity where 0xFF is fully on and 0x00 is fully off.
  u_int16_t bottom_pwm_level;       // PWM duty cycle for the UV illumination source on bottom level.
  u_int32_t encryption_key;         // Set to 0, or else layers are encrypted.
  u_int32_t slicer_config_offset;   // offset in file for second extended configuration.
  u_int32_t slicer_config_size;     // size of the second extended configuration, in bytes.
} ctb_headers_t;

typedef struct {
  float bottom_lift_distance;       // distance to lift the build platform away from the vat for bottom layers, in mm.
  float bottom_lift_speed;          // speed at which to lift the build platform away from the vat after bottom layer, in mm/min
  float lift_distance;              // distance to lift the build platform away from the vat, in mm.
  float lift_speed;                 // speed at which to lift the build platform away from the vat, in mm/min.
  float retract_speed;              // speed to use when the build platform re-approaches the vat after lift, mm/min.
  float resin_volume;               // estimated resin needed, in ml.
  float resin_mass;                 // estimated resin needed, in g.
  float resin_cost;                 // estimated resin needed, in your currency.
  float bottom_light_off_time;      // light off time setting used at slicing, in secs for bottom layers.
                                    // Overriden in layer information.
  float light_off_time;             // light off time setting used at slicing, in secs. Overriden in layer information.
                                    // Also in ext_config.
  u_int32_t bottom_layer_count;     // Number of layer being considered bottom. Also in ext_config.
  u_int32_t padding1;
  u_int32_t padding2;
  u_int32_t padding3;
  u_int32_t padding4;
  u_int32_t padding5;
} ctb_print_config_t;

typedef struct {
  float bottom_lift_distance2;
  float bottom_lift_speed2;
  float lift_distance2;
  float lift_speed2;
  float retract_distance2;
  float retract_speed2;
  float reset_time_after_lift;
  u_int32_t machine_type_offset;    // file offset to a string naming the machine type.
  u_int32_t machine_type_len;       // the length of the name string (not null terminated).
  u_int32_t per_layer_settings;     // 0xF = per layer settings disabled, 0x2000000F = enabled (v3), 0x4000000F = enabled (v4).
  u_int32_t mysterious_id;
  u_int32_t antialias_level;
  u_int32_t software_version;
  float rest_time_after_retract;
  float rest_time_after_lift2;
  u_int32_t transition_layer_count;
  u_int32_t print_config_v4_offset;
  u_int32_t padding1;
  u_int32_t padding2;
} ctb_slicer_config_t;

typedef struct {
  float bottom_retract_speed;
  float bottom_retract_speed2;
  u_int32_t padding1;
  float four1;
  u_int32_t padding2;
  float four2;
  float rest_time_after_retract;
  float rest_time_after_lift;
  float rest_time_before_lift;
  float bottom_retract_height2;
  float unknown1;
  u_int32_t unknown2;
  u_int32_t unknown3;
  u_int32_t last_layer_index;
  u_int32_t padding3;
  u_int32_t padding4;
  u_int32_t padding5;
  u_int32_t padding6;
  u_int32_t disclaimer_offset;
  u_int32_t disclaimer_len;
} ctb_print_config_v4_t;

typedef struct {
  u_int32_t resolution_x;
  u_int32_t resolution_y;
  u_int32_t image_offset;
  u_int32_t image_length;
  u_int32_t unknown1;
  u_int32_t unknown2;
  u_int32_t unknown3;
  u_int32_t unknown4;
} ctb_preview_t;

typedef struct {
  float z;
  float exposure;       // in seconds
  float light_off_time; // in seconds
  u_int32_t data_offset;
  u_int32_t data_len;
  u_int32_t unknown1;
  u_int32_t table_size;
  u_int32_t unknown2;
  u_int32_t unknown3;
} ctb_layer_header_base_t;

typedef struct {
  u_int32_t total_size;
  float lift_distance;
  float lift_speed;
  float lift_distance2;
  float lift_speed2;
  float retract_speed;
  float retract_distance2;
  float retract_speed2;
  float rest_time_before_lift;
  float rest_time_after_lift;
  float rest_time_after_retract;
  float light_pwm;
} ctb_layer_header_extended_t;

typedef struct {
  ctb_layer_header_base_t base;
  ctb_layer_header_extended_t extended;
} ctb_layer_header_t;

typedef struct {
  ctb_headers_t headers;
  ctb_print_config_t print_config;
  ctb_slicer_config_t slicer_config;
  ctb_print_config_v4_t print_config_v4;
  ctb_preview_t large_preview;
  ctb_preview_t small_preview;
  ctb_layer_header_t *layer_headers;
  char *file_path;
  char *machine_name;
  char *disclaimer;
  char reserved[384];
} ctb_t;

typedef struct {
  u_int8_t blue:5;
  u_int8_t run_flag:1;
  u_int8_t green:5;
  u_int8_t red:5;
} pixel_t;

sl1_t *   new_sl1             ();
void      free_sl1            (sl1_t *s);
ctb_t *   new_ctb             ();
void      free_ctb            (ctb_t *c);
int       parse_sl1_file      (sl1_t *sl1, const char *in);
int       parse_ctb_file      (ctb_t *ctb, const char *in);
int       read_preview_file   (u_int8_t **data, size_t *len, const ctb_t *c, size_t type);
void      decrypt_layer       (u_int8_t *raw_data, u_int32_t encryption_key, u_int32_t layer_index, size_t len);
int       decode_layer        (u_int8_t **data, size_t *data_len, const u_int8_t *raw_data, size_t raw_len, const ctb_t *ctb, size_t *nonzero_pixels_count);


#endif
