
#include <stdint.h>
#include "math.h"
#include "control.h"//get control output
#include <rtwtypes.h>
#include "stm32h7xx_hal.h" //needed in order to reach HAL timer handlers, dictionary for microcontroller that defines periperals


extern ExtU rtU;
extern ExtY rtY;// Simulink output
extern TIM_HandleTypeDef htim2;//from main
extern TIM_HandleTypeDef htim3;//from main
#define MAX_BLDC_PWM 65535// 16-bit resolution
#define MAX_BLDC_VOLTAGE 24.0

void set_bldc_pwm(void){
    for(int i = 0; i < 6; i++){
        uint32_t pwm_value = (uint32_t)((rtY.controlb[i] / MAX_BLDC_VOLTAGE) * MAX_BLDC_PWM); //changes duty cycle (percentage of times its on, higher -> motor spins faster)
                                                                                        //idk if this is the formula for scaling will ask control
        if (pwm_value>MAX_BLDC_PWM){
            pwm_value=MAX_BLDC_PWM;
        }
        if (pwm_value<0){
            pwm_value=0;
        }
        switch(i)
        {
            case 0: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_value); break;
            case 1: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm_value); break;
            case 2: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm_value); break;
            case 3: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_value); break;
            case 4: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm_value); break;
            case 5: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pwm_value); break;
        }
    }
    
}