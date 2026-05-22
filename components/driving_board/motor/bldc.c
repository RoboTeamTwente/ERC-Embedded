
#include <stdint.h>
#include "math.h"
#include "control_drive.h"//get control output
#include <rtwtypes.h>
#include "stm32h7xx_hal.h" //needed in order to reach HAL timer handlers, dictionary for microcontroller that defines periperals
#include "bldc.h"
#include "cmsis_os2.h"

extern ExtU rtU;
extern ExtY rtY;// Simulink output
extern TIM_HandleTypeDef htim1;//from main
extern TIM_HandleTypeDef htim3;//from main


// DIR pin
#define DIR_PORT GPIOA
#define DIR_PIN  GPIO_PIN_1

#define MIN_FREQ  200
#define MAX_FREQ  20000
#define STEP_FREQ 50
#define STEP_DELAY_MS 2

//--------------------------------------------------
// Set pulse frequency
//--------------------------------------------------
void CL3E_SetFrequency(uint32_t freq)
{
    uint32_t timer_clk = HAL_RCC_GetPCLK2Freq() * 2;

    if(freq < MIN_FREQ) freq = MIN_FREQ;
    if(freq > MAX_FREQ) freq = MAX_FREQ;

    uint32_t arr = (timer_clk / freq) - 1;

    if(arr < 10) arr = 10;

    __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, arr / 2); //%50 duty cycle
}

void CL3E_RampTo(uint32_t target)
{
    static uint32_t current = 500;

    if(target == current)
        return;

    //--------------------------------------------------
    // Ramp up
    //--------------------------------------------------
    if(target > current)
    {
        for(uint32_t f = current; f <= target; f += STEP_FREQ)
        {
            CL3E_SetFrequency(f);
            current = f;
            osDelay(STEP_DELAY_MS);
        }
    }
    //--------------------------------------------------
    // Ramp down 
    //--------------------------------------------------
    else
    {
        for(int32_t f = (int32_t)current;
            f >= (int32_t)target;
            f -= STEP_FREQ)
        {
            CL3E_SetFrequency((uint32_t)f);
            current = (uint32_t)f;
            osDelay(STEP_DELAY_MS);
        }
    }

    current = target;
}

//--------------------------------------------------
// Forward 
//--------------------------------------------------
void CL3E_Forward(uint32_t freq)
{
    HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_SET);
    CL3E_RampTo(freq);
}

//--------------------------------------------------
// Backward
//--------------------------------------------------
void CL3E_Backward(uint32_t freq)
{
    HAL_GPIO_WritePin(DIR_PORT, DIR_PIN, GPIO_PIN_RESET);
    CL3E_RampTo(freq);
}
//--------------------------------------------------
// Stop
//--------------------------------------------------
void CL3E_Stop(void)
{
    CL3E_RampTo(MIN_FREQ);

    osDelay(50);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

void CL3E_Test(void)
{
    CL3E_Forward(500);
    osDelay(2000);

    CL3E_Forward(3000);
    osDelay(2000);

    CL3E_Stop();
    osDelay(300);

    CL3E_Backward(3000);
    osDelay(2000);

    CL3E_Backward(500);
    osDelay(2000);

    CL3E_Stop();
    osDelay(1000);
}


/**
 * #define MAX_BLDC_PWM 65535.0f// 16-bit resolution
#define MAX_BLDC_VOLTAGE 24.0f

void set_motor_direction(GPIO_TypeDef *port, uint16_t pin, uint8_t dir)
{
    if(dir)
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}


void set_bldc_pwm()
{
    // LEFT FRONT
    set_motor_direction(LF_DIR_PORT, LF_DIR_PIN,
                        (rtY.controlLF >= 0));

    set_single_bldc_pwm(&htim1, 1, rtY.controlLF);

    // LEFT MID
    set_motor_direction(LM_DIR_PORT, LM_DIR_PIN,
                        (rtY.controlLM >= 0));

    set_single_bldc_pwm(&htim1, 2, rtY.controlLM);

    // LEFT BACK
    set_motor_direction(LB_DIR_PORT, LB_DIR_PIN,
                        (rtY.controlLB >= 0));

    set_single_bldc_pwm(&htim1, 3, rtY.controlLB);

    // RIGHT FRONT
    set_motor_direction(RF_DIR_PORT, RF_DIR_PIN,
                        (rtY.controlRF >= 0));

    set_single_bldc_pwm(&htim1, 4, rtY.controlRF);

    // RIGHT MID
    set_motor_direction(RM_DIR_PORT, RM_DIR_PIN,
                        (rtY.controlRM >= 0));

    set_single_bldc_pwm(&htim3, 1, rtY.controlRM);

    // RIGHT BACK
    set_motor_direction(RB_DIR_PORT, RB_DIR_PIN,
                        (rtY.controlRB >= 0));

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
    

// TEST 1 : FORWARD ONLY
// ------------------------------------------------------

void motor_test_forward(void)
{
    set_motor_direction(LF_DIR_PORT, LF_DIR_PIN, 1);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 15000);

    osDelay(3000);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

    osDelay(1000);
}



// TEST 2 : REVERSE ONLY
// ------------------------------------------------------

void motor_test_reverse(void)
{
    set_motor_direction(LF_DIR_PORT, LF_DIR_PIN, 0);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 15000);

    osDelay(3000);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

    osDelay(1000);
}



// TEST 3 : SPEED RAMP FORWARD + REVERSE
// ------------------------------------------------------

void motor_test_ramp(void)
{
    // ---------------- FORWARD ----------------

    set_motor_direction(LF_DIR_PORT, LF_DIR_PIN, 1);

    for(int pwm = 0; pwm <= 30000; pwm += 2000)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);
        osDelay(100);
    }

    osDelay(1000);

    for(int pwm = 30000; pwm >= 0; pwm -= 2000)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);
        osDelay(100);
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

    osDelay(1000);

    // ---------------- REVERSE ----------------

    set_motor_direction(LF_DIR_PORT, LF_DIR_PIN, 0);

    for(int pwm = 0; pwm <= 30000; pwm += 2000)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);
        osDelay(100);
    }

    osDelay(1000);

    for(int pwm = 30000; pwm >= 0; pwm -= 2000)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm);
        osDelay(100);
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

    osDelay(2000);
}


 */
