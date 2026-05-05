
#include <stdint.h>
#include "math.h"
#include "control_drive.h"//get control output
#include <rtwtypes.h>
#include "stm32h7xx_hal.h" //needed in order to reach HAL timer handlers, dictionary for microcontroller that defines periperals


extern ExtU rtU;
extern ExtY rtY;// Simulink output
extern TIM_HandleTypeDef htim1;//from main
extern TIM_HandleTypeDef htim3;//from main
#define MAX_BLDC_PWM 65535.0f// 16-bit resolution
#define MAX_BLDC_VOLTAGE 24.0f


void set_bldc_pwm(){
    set_single_bldc_pwm(&htim1, 1, rtY.controlLF);
    set_single_bldc_pwm(&htim1, 2, rtY.controlLM);
    set_single_bldc_pwm(&htim1, 3, rtY.controlLB);
    set_single_bldc_pwm(&htim1, 4, rtY.controlRF);
    set_single_bldc_pwm(&htim3, 1, rtY.controlRM);
    set_single_bldc_pwm(&htim3, 2, rtY.controlRB);
}

void set_single_bldc_pwm(TIM_HandleTypeDef *htim, int tim_channel_no, real_T control_val){
    uint32_t pwm_value = (uint32_t)(((double)fabs(control_val) / MAX_BLDC_VOLTAGE) * MAX_BLDC_PWM); //changes duty cycle (percentage of times its on, higher -> motor spins faster)
                                                                                        //idk if this is the formula for scaling will ask control
        if (pwm_value>MAX_BLDC_PWM){
            pwm_value=MAX_BLDC_PWM;
        }
        if (pwm_value<0){
            pwm_value=0;
        }
        switch(tim_channel_no)
        {
            case 1: __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, pwm_value); break;
            case 2: __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, pwm_value); break;
            case 3: __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, pwm_value); break;
            case 4: __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_4, pwm_value); break;
        }
    }
    


/**
 *             case 1: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_value); break;
            case 2: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_value); break;
            case 3: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_value); break;
            case : __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_value); break;
            case 4: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm_value); break;
            case 5: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pwm_value); break;
 */