#ifndef _PARSER_H
#define _PARSER_H

#include <stdint.h>

enum {
  PREVIEW_LARGE,
  PREVIEW_SMALL,
};

enum {
  SL1_ORIENTATION_LANDSCAPE,
  SL1_ORIENTATION_PORTRAIT,
};

typedef struct {
  char *file_path;
  char *base_name;
  char *bed_shape;
  double bottle_cost;
  double bottle_volume;
  double bottle_weight;
  char *default_sla_material_profile;
  char *default_sla_print_profile;
  double display_height;
  bool display_mirror_x;
  bool display_mirror_y;
  int display_orientation;
  int display_pixels_x;
  int display_pixels_y;
  int display_width;
  double exposure_time;
  int faded_layers;
  int fast_tilt_time;
  int initial_exposure_time;
  double initial_layer_height;
  double layer_height;
  double max_print_height;
  char *printer_notes;
  char *printer_settings_id;
  char *sla_material_settings_id;
  char *sla_print_settings_id;
  int num_fast_layers;
  int num_slow_layers;
  int print_time;
} sl1_t;

typedef struct {
  uint32_t magic;                   // 0x12fd0086 for ctb files.
  uint32_t version;                 // ctb file version. We support 3 and 4.
  float bed_size_x;                 // length of the build plate, in mm.
  float bed_size_y;                 // width of the build plate, in mm.
  float bed_size_z;                 // height of the build plate, in mm.
  uint32_t unknown1;
  uint32_t unknown2;
  float total_height;               // height of the printed object, in mm.
  float layer_height;               // height of each layer, in mm. Overriden in layer information.
  float layer_exposure;             // exposure time for normal layers, in secs. Overriden in layer information.
  float bottom_exposure;            // exposure time for bottom layers, in secs. Overriden in layer information.
  float light_off_delay;            // light off time setting used at slicing, in secs. Overriden in layer information.
                                    // Also in ext_config.
  uint32_t bottom_layer_count;      // Number of layer being considered bottom. Also in ext_config.
  uint32_t resolution_x;            // Printer resolution on x axis, in px.
  uint32_t resolution_y;            // Printer resolution on y axis, in px.
  uint32_t large_preview_offset;    // offset in file for large preview image headers.
  uint32_t layer_table_offset;      // offset in file for layer headers.
  uint32_t layer_count;             // number of layers in the print.
  uint32_t small_preview_offset;    // offset in file for small preview image headers.
  uint32_t print_time;              // estimated print time, in secs.
  uint32_t projection;              // 0 (normal) or 1 (mirrored)
  uint32_t print_config_offset;     // offset in file for extended configuration.
  uint32_t print_config_size;       // size of the extended configuration, in bytes.
  uint32_t antialias_level;         // unused (legacy from cbddlp). Set to 1.
  uint16_t pwm_level;               // PWM duty cycle for the UV illumination source on normal level.
                                    // This appears to be an 8-bit quantity where 0xFF is fully on and 0x00 is fully off.
  uint16_t bottom_pwm_level;        // PWM duty cycle for the UV illumination source on bottom level.
  uint32_t encryption_key;          // Random value, used as seed for encryption.
  uint32_t slicer_config_offset;    // offset in file for second extended configuration.
  uint32_t slicer_config_size;      // size of the second extended configuration, in bytes.
} ctb_headers_t;

typedef struct {
  float bottom_lift_height;         // distance to lift the build platform away from the vat for bottom layers, in mm.
  float bottom_lift_speed;          // speed at which to lift the build platform away from the vat after bottom layer, in mm/min
  float lift_height;                // distance to lift the build platform away from the vat, in mm.
  float lift_speed;                 // speed at which to lift the build platform away from the vat, in mm/min.
  float retract_speed;              // speed to use when the build platform re-approaches the vat after lift, mm/min.
  float resin_volume;               // estimated resin needed, in ml.
  float resin_mass;                 // estimated resin needed, in g.
  float resin_cost;                 // estimated resin needed, in your currency.
  float bottom_light_off_delay;     // light off time setting used at slicing, in secs for bottom layers.
                                    // Overriden in layer information.
  float light_off_delay;            // light off time setting used at slicing, in secs. Overriden in layer information.
                                    // Also in ext_config.
  uint32_t bottom_layer_count;      // Number of layer being considered bottom. Also in ext_config.
  uint32_t padding1;
  uint32_t padding2;
  uint32_t padding3;
  uint32_t padding4;
} ctb_print_config_t;

typedef struct {
  float bottom_lift_height2;
  float bottom_lift_speed2;
  float lift_height2;
  float lift_speed2;
  float retract_distance2;
  float retract_speed2;
  float reset_time_after_lift;
  uint32_t machine_type_offset;     // file offset to a string naming the machine type.
  uint32_t machine_type_len;        // the length of the name string (not null terminated).
  uint32_t per_layer_settings;      // 0xF = per layer settings disabled, 0x2000000F = enabled (v3), 0x4000000F = enabled (v4).
  uint32_t mysterious_id;
  uint32_t antialias_level;
  uint32_t software_version;
  float rest_time_after_retract;
  float rest_time_after_lift2;
  uint32_t transition_layer_count;
  uint32_t print_config_v4_offset;
  uint32_t padding1;
  uint32_t padding2;
} ctb_slicer_config_t;

typedef struct {
  float bottom_retract_speed;
  float bottom_retract_speed2;
  uint32_t padding1;
  float four1;
  uint32_t padding2;
  float four2;
  float rest_time_after_retract;
  float rest_time_after_lift;
  float rest_time_before_lift;
  float bottom_retract_height2;
  float unknown1;
  uint32_t unknown2;
  uint32_t unknown3;
  uint32_t last_layer_index;
  uint32_t padding3;
  uint32_t padding4;
  uint32_t padding5;
  uint32_t padding6;
  uint32_t disclaimer_offset;
  uint32_t disclaimer_len;
  uint32_t reserved[96];
} ctb_print_config_v4_t;

typedef struct {
  uint32_t resolution_x;
  uint32_t resolution_y;
  uint32_t image_offset;
  uint32_t image_length;
  uint32_t unknown1;
  uint32_t unknown2;
  uint32_t unknown3;
  uint32_t unknown4;
} ctb_preview_t;

typedef struct {
  float z;
  float exposure_time;  // in seconds
  float light_off_delay; // in seconds
  uint32_t data_offset;
  uint32_t data_len;
  uint32_t unknown1;
  uint32_t table_size;
  uint32_t unknown2;
  uint32_t unknown3;
} ctb_layer_header_base_t;

typedef struct {
  float z;
  float exposure_time;  // in seconds
  float light_off_delay; // in seconds
  uint32_t data_offset;
  uint32_t data_len;
  uint32_t unknown1;
  uint32_t table_size;
  uint32_t unknown2;
  uint32_t unknown3;

  uint32_t total_size;
  float lift_height;
  float lift_speed;
  float lift_height2;
  float lift_speed2;
  float retract_speed;
  float retract_height2;
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
  // file structure
  ctb_headers_t headers;
  ctb_preview_t large_preview;
  ctb_preview_t small_preview;
  ctb_print_config_t print_config;
  ctb_slicer_config_t slicer_config;
  char *machine_name;
  char *v4_disclaimer;
  ctb_print_config_v4_t print_config_v4;
  ctb_layer_header_t *layer_headers;

  // not in the file
  char *file_path;
} ctb_t;

sl1_t *   new_sl1                   ();
void      free_sl1                  (sl1_t *s);
ctb_t *   new_ctb                   ();
void      free_ctb                  (ctb_t *c);
bool      is_sl1_file               (const char *in);
bool      is_ctb_file               (const char *in);
int       parse_sl1_archive         (sl1_t *sl1, const char *in);
int       parse_ctb_file            (ctb_t *ctb, const char *in);
int       ctb_read_preview_file     (uint8_t **data, size_t *len, const ctb_t *c, size_t type);
int       sl1_read_preview_file     (uint8_t **data, size_t *len, const sl1_t *s, size_t type);
int       sl1_read_layer_image_file (uint8_t **data, size_t *len, const sl1_t *s, size_t layer_index);
void      decrypt_layer             (uint8_t *raw_data, uint32_t encryption_key, uint32_t layer_index, size_t len);
int       decode_layer              (uint8_t **data, size_t *data_len, const uint8_t *raw_data, size_t raw_len,
                                     const ctb_t *ctb, size_t *nonzero_pixels_count);


#endif
