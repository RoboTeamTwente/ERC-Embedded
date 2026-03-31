#include "pressure_sensor.h"

#include <string.h>

result_t pressure_sensor_init(pressure_sensor_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  memset(data, 0, sizeof(*data));
  data->is_calibrated = false;
  return RESULT_OK;
}

result_t poll_pressure_sensor(pressure_sensor_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  (void)data;
  return RESULT_ERR_UNIMPLEMENTED;
}

result_t pressure_sensor_get_pressure_kpa(const pressure_sensor_data_t *data,
                                          float *pressure_kpa) {
  if (data == NULL || pressure_kpa == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *pressure_kpa = data->pressure_kpa;
  return RESULT_OK;
}

result_t pressure_sensor_get_temperature_c(const pressure_sensor_data_t *data,
                                           float *temperature_c) {
  if (data == NULL || temperature_c == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *temperature_c = data->temperature_c;
  return RESULT_OK;
}

result_t pressure_sensor_get_voltage(const pressure_sensor_data_t *data,
                                     float *voltage) {
  if (data == NULL || voltage == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *voltage = data->voltage;
  return RESULT_OK;
}

result_t pressure_sensor_is_valid(const pressure_sensor_data_t *data,
                                  bool *is_valid) {
  if (data == NULL || is_valid == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *is_valid = data->is_calibrated;
  return RESULT_OK;
}
