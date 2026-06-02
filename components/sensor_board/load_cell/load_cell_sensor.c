#include "load_cell_sensor.h"

#include <string.h>

result_t load_cell_sensor_init(load_cell_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  memset(data, 0, sizeof(*data));
  data->is_calibrated = false;
  return RESULT_OK;
}

result_t poll_load_cell_sensor(load_cell_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  (void)data;
  return RESULT_ERR_UNIMPLEMENTED;
}

result_t load_cell_get_force_newtons(const load_cell_data_t *data,
                                     float *force_newtons) {
  if (data == NULL || force_newtons == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *force_newtons = data->force_newtons;
  return RESULT_OK;
}

result_t load_cell_get_mass_grams(const load_cell_data_t *data,
                                  float *mass_grams) {
  if (data == NULL || mass_grams == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *mass_grams = data->mass_grams;
  return RESULT_OK;
}

result_t load_cell_get_raw_counts(const load_cell_data_t *data,
                                  int32_t *raw_counts) {
  if (data == NULL || raw_counts == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *raw_counts = data->raw_counts;
  return RESULT_OK;
}

result_t load_cell_get_calibration(const load_cell_data_t *data,
                                   float *scale_newtons_per_count,
                                   int32_t *tare_offset_counts) {
  if (data == NULL || scale_newtons_per_count == NULL ||
      tare_offset_counts == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *scale_newtons_per_count = data->scale_newtons_per_count;
  *tare_offset_counts = data->tare_offset_counts;
  return RESULT_OK;
}

result_t load_cell_sensor_is_valid(const load_cell_data_t *data, bool *is_valid) {
  if (data == NULL || is_valid == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *is_valid = data->is_calibrated;
  return RESULT_OK;
}
