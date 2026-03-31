#include "stdint.h"
#include "result.h"
#include "stepper.h"
#include "tim.h"


//Alledgedly, the steppers will be using CL57T drivers!
/*  CL57T driver
    Compatible with NEMA23 motor
    DEFAULT microstep resolution: 1600
    DEFAULT direction: counter clockwise
    ((I *think* that that means that 1 is CW and 0 is CCW))
    DEFAULT pulse mode: single pulse

    Pulse width should always be greater than 2.5 microsecs!!
*/

//Placeholder pins
#define PLS_PIN 2 //Pulse signal
#define DIR_PIN 3 //Direction signal
#define ENA_PIN 4 //Enable signal
#define ALM_PIN 5 //Alarm signal (OUT)

//NOTE: placeholder values
#define PWM_FREQ 1000
#define DUTY_CYCLE 50

#define STEPS_PER_REV 200 //In steps / rev
#define MICRO_STEP 16
#define TOTAL_STEP STEPS_PER_REV * MICRO_STEP
#define DEGREE_PER_MICROSTEP 360 / TOTAL_STEP

#define RPM 100

TIM_HandleTypeDef htim2;

int32_t ARR = 65535;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_TIM2_Init(void);


result_t init_stepper() {
    set_pin(ENA_PIN, "HIGH");
    stepper.current_angle = 0;
    stepper.pwm = 0;
}

uint32_t stepper_make_steps(uint32_t angle) {
    angle = angle % 360;
    uint32_t steps_for_angle = angle / DEGREE_PER_MICROSTEP;

    uint32_t seconds_for_angle = steps_for_angle / PWM_FREQ;
    
    return seconds_for_angle;
}

void rotate_stepper(uint32_t target_angle) {

    uint32_t current_angle = stepper.current_angle;
    uint32_t CW_angle = target_angle - current_angle;
    uint32_t CCW_angle = 360 - CW_angle;

    uint32_t seconds;
    if (CW_angle < CCW_angle) {
        seconds = stepper_make_steps(CW_angle);
        set_pin(DIR_PIN, 1); //1 for clockwise, 0 for counterclockwise
    } else if (CCW_angle < CW_angle) {
        seconds = stepper_make_steps(CCW_angle);
        set_pin(DIR_PIN, 0); //1 for clockwise, 0 for counterclockwise
    }
    stepper.current_angle = target_angle;

    do_pwm();
}

//NOTE: HAVE TO SET PIN TO GPIO OUT
void do_pwm_manual() {

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    float frequency = 100; //total length of up/down wave (in us)
    float dutyCycle = 0.5; //percentage of up compared to down

    while (1) {
        // PIN ON
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
        HAL_Delay(frequency*dutyCycle);
        // PIN OFF
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_Delay(frequency*(1-dutyCycle));
    }

}

//NOTE: HAVE TO SET DMA
void do_pwm_dma() {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
  
	float DutyCycle = 0.5;
	int32_t CH1_DC = DutyCycle * ARR;

    // Sawtooth wave PWM duty cycle (0 to 1599)
    uint16_t sawtooth_duty[5] = {0*ARR, 0.25*ARR, 0.5*ARR, 0.75*ARR, ARR};
    
    HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)&sawtooth_duty,5);

    // while (1) {
    // 	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, CH1_DC);
    // 	HAL_Delay(1);
    // }
}

void do_pwm() {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

	float DutyCycle = 0.5;
	int32_t CH1_DC = DutyCycle * ARR;

    while (1) {
    	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, CH1_DC);
    	HAL_Delay(1);
    }
}

void do_pwm_timed() {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    // Setup slave timer
    TIM1->CCER = TIM_CCER_CC1E; // Enable OC1 output
    TIM1->BDTR = TIM_BDTR_MOE; // Needed for timers with break functionality
    TIM1->ARR = 50; //us period value
    TIM1->CCR1 = (TIM1->ARR / 2) + 1; // 50% duty cycle
    TIM1->CCMR1 = (7 << TIM_CCMR1_OC1M_Pos); // Inverted PWM mode (pulse is right justified inside period)
    TIM1->SMCR = (1 << TIM_SMCR_TS_Pos) | (5 << TIM_SMCR_SMS_Pos); // Use internal trigger 1 (TIM2), slave gated mode
    TIM1->CR1 = TIM_CR1_CEN; // Enable counter (it will not start counting immediately, as it waits for the signal from master timer)

    // Setup master timer
    TIM2->ARR = 50; //ms period value
    TIM2->CR1 = TIM_CR1_OPM; // One pulse mode
    TIM2->CR2 = (1 << TIM_CR2_MMS_Pos); // Master mode, trig out = counter enable
    TIM2->CR1 |= TIM_CR1_CEN; // Enable counter, this signals slave counter to start and thus produce pulses

     TIM1->CNT = 0; // Reset the slave counter, as it might be slightly over 0 after automatic disable
    // TIM2->ARR = Optionally change for how long to produce pulses
    TIM2->CR1 |= TIM_CR1_CEN; // Re-enable counters

}

void delay_by_rpm() {
    int ms_in_minute = 60000000;
    delayMicroseconds(ms_in_minute/STEPS_PER_REV/RPM);
}

void set_pin(int pinname, char what) {
    return;
}

