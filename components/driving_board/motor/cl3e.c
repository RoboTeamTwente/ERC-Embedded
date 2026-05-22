
#include <stdint.h>
#include "math.h"
#include "control_drive.h"//get control output
#include <rtwtypes.h>
#include "stm32h7xx_hal.h" //needed in order to reach HAL timer handlers, dictionary for microcontroller that defines periperals
#include "cl3e.h"
#include "cmsis_os2.h"
#include "result.h"


extern ExtU rtU;
extern ExtY rtY;// Simulink output
extern TIM_HandleTypeDef htim1;//from main
extern TIM_HandleTypeDef htim3;//from main


// DIR pin
//#define DIR_PORT GPIOA
//#define DIR_PIN  GPIO_PIN_3

#define MIN_FREQ  200
#define MAX_FREQ  15000
//#define STEP_FREQ 50
//#define STEP_DELAY_MS 2
#define MAX_BLDC_VOLTAGE 24.0f

uint32_t CL3E_GetTimerClock(TIM_HandleTypeDef *htim)
{
    uint32_t pclk;

    // APB2 timers
    if(htim->Instance == TIM1  ||
       htim->Instance == TIM8  ||
       htim->Instance == TIM15 ||
       htim->Instance == TIM16 ||
       htim->Instance == TIM17)
    {
        pclk = HAL_RCC_GetPCLK2Freq();
    }
    // APB1 timers
    else
    {
        pclk = HAL_RCC_GetPCLK1Freq();
    }

    return (pclk * 2) / (htim->Init.Prescaler + 1);
}

void CL3E_SetFrequency(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq)
{
    uint32_t timer_clk = CL3E_GetTimerClock(htim);


    if(freq < MIN_FREQ) freq = MIN_FREQ;
    if(freq > MAX_FREQ) freq = MAX_FREQ;

    uint32_t arr = (timer_clk / freq) - 1;

    if(arr < 10) arr = 10;

    __HAL_TIM_SET_AUTORELOAD(htim, arr);
    __HAL_TIM_SET_COUNTER(htim, 0);

    __HAL_TIM_SET_COMPARE(htim, channel, arr / 2);

}

uint32_t CL3E_ControlToFreq(real_T u)
{
    double norm = fabs(u) / MAX_BLDC_VOLTAGE;

    if (norm > 1.0) norm = 1.0;

    return (uint32_t)(MIN_FREQ + norm * (MAX_FREQ - MIN_FREQ));
}

void CL3E_DriveFromControl(TIM_HandleTypeDef *htim, uint32_t channel, GPIO_TypeDef *dir_port,
                           uint16_t dir_pin, real_T u)
{
    // direction
    if (u >= 0)
        HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(dir_port, dir_pin, GPIO_PIN_RESET);

    // stop condition
    if (fabs(u) < 0.1)
    {
        __HAL_TIM_SET_COMPARE(htim, channel, 0);
        return;
    }

    // frequency obtained from scaling control signal
    uint32_t freq = CL3E_ControlToFreq(u);

    CL3E_SetFrequency(htim, channel, freq);
}
