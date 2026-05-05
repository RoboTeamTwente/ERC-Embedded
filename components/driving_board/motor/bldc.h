#ifndef BLDC_H
#define BLDC_H

#include <stdint.h>

void set_bldc_pwm(void);

void set_single_bldc_pwm(TIM_HandleTypeDef *htim, int tim_channel_no, real_T control_val);

#endif