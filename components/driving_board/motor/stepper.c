
#include <stdint.h>
#include "math.h"
#include "control.h"//get control output
#include <rtwtypes.h>
#include "stm32h7xx_hal.h" //needed in order to reach HAL timer handlers, dictionary for microcontroller that defines periperals


extern ExtU rtU;
extern ExtY rtY;// Simulink output
extern TIM_HandleTypeDef htim2;//from main
extern TIM_HandleTypeDef htim3;//from main

void set_stepper_pwm(void){
    for(int i = 0; i < 4; i++){
        switch(i)
        {
            case 0: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, rtY.pwnenable[i]); break;
            case 1: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, rtY.pwnenable[i]); break;
            case 2: __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, rtY.pwnenable[i]); break;
            case 3: __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, rtY.pwnenable[i]); break;
        }
    }
    
}