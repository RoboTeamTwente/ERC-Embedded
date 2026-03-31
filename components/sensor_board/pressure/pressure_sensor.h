#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include "result.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float pressure_kpa;
  float temperature_c;
  float voltage;
  bool is_calibrated;
} pressure_sensor_data_t;

result_t pressure_sensor_init(pressure_sensor_data_t *data);
result_t poll_pressure_sensor(pressure_sensor_data_t *data);

result_t pressure_sensor_get_pressure_kpa(const pressure_sensor_data_t *data,
                                          float *pressure_kpa);
result_t pressure_sensor_get_temperature_c(const pressure_sensor_data_t *data,
                                           float *temperature_c);
result_t pressure_sensor_get_voltage(const pressure_sensor_data_t *data,
                                     float *voltage);
result_t pressure_sensor_is_valid(const pressure_sensor_data_t *data,
                                  bool *is_valid);

#endif
