#ifndef SCALE_LOGIC_H
#define SCALE_LOGIC_H

typedef struct {
    float raw_data;
    float calibration_factor;
} scale_t;

void init();

void tare();

void calibrate();

#endif SCALE_LOGIC_H;