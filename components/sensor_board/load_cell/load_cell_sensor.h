#ifndef LOAD_CELL_SENSOR_H
#define LOAD_CELL_SENSOR_H

#include "result.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int32_t raw_counts;
  float force_newtons;
  float mass_grams;
  float scale_newtons_per_count;
  int32_t tare_offset_counts;
  bool is_calibrated;
} load_cell_data_t;

result_t load_cell_sensor_init(load_cell_data_t *data);
result_t poll_load_cell_sensor(load_cell_data_t *data);

result_t load_cell_get_force_newtons(const load_cell_data_t *data,
                                     float *force_newtons);
result_t load_cell_get_mass_grams(const load_cell_data_t *data,
                                  float *mass_grams);
result_t load_cell_get_raw_counts(const load_cell_data_t *data,
                                  int32_t *raw_counts);
result_t load_cell_get_calibration(const load_cell_data_t *data,
                                   float *scale_newtons_per_count,
                                   int32_t *tare_offset_counts);
result_t load_cell_sensor_is_valid(const load_cell_data_t *data, bool *is_valid);

#endif
