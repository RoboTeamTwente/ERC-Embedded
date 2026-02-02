#ifndef PIO_UNIT_TESTING

#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "control.h"
#include "cubemx_main.h"
#include "driving_board.pb.h"
#include "gpio.h"
#include "logging.h"
#include "pb_message.h"
#include "rtwtypes.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bldc.h"
//#include "parser.h"


#include "calculator.h"

static char *TAG = "MAIN";

extern ExtU rtU;
extern ExtY rtY;

void Error_Handler(void);
void cubemx_main(void);
void SystemClock_Config(void);
void MPU_Config(void);
void MX_GPIO_Init(void);
//void MX_DAC1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
//DAC_HandleTypeDef hdac1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

//uint16_t dac_value;
void MainTask(void *argument);
void PwmTask(void *argument);
void DrivingSensorTask(void *argument);


// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t pwmTask_attributes = {
    .name = "pwmTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t drivingSensorTask_attributes = {
    .name = "pwmTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

void init_board() {
  HAL_Init();
  SystemClock_Config();

  osKernelInitialize();
  MX_GPIO_Init();
  //MX_DAC1_Init();
  //HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);

  /* Initialize COM1 port */
  BspCOMInit.BaudRate = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits = COM_STOPBITS_1;
  BspCOMInit.Parity = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
    Error_Handler();
  }
  MX_USART3_Init(&huart_com, &BspCOMInit);
  LOG_init(&huart_com);
  MX_TIM2_Init();
  MX_TIM3_Init();
  control_initialize();

  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osThreadNew(PwmTask, NULL, &pwmTask_attributes);
  osThreadNew(DrivingSensorTask, NULL, &drivingSensorTask_attributes);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

  osKernelStart();


  while (1) {
  }
}

/**

void pwm_test(){//gradually increases decreases pwm duty cycle
  
  int32_t CH1_DC = 0;

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  while (1)
  {
    while(CH1_DC < 65535)
    {
    	  TIM2->CCR1 = CH1_DC;
    	  CH1_DC += 70;
    	  HAL_Delay(1);
    }
    while(CH1_DC > 0)
    {
        TIM2->CCR1 = CH1_DC;//were writing directly to hardware register so method for updating pwm is not needed
        CH1_DC -= 70;
        HAL_Delay(1);
    }
  
    }
  }

 * 
 */

int main(void) { init_board(); }

/**
 * @brief  Main application task
 * @param  argument: Not used
 * @retval None
 */
void MainTask(void *argument) {//send messages calculates actual values from reallife sensors

  //BSP_LED_Init(LED_GREEN);
  //BSP_LED_Init(LED_BLUE);
  //BSP_LED_Init(LED_RED);


  //BSP_LED_Toggle(LED_GREEN);

for (size_t i = 0; i < 4; i++)//stub values actual will come from decode
   {
      rtU.actang[i] = rtY.desang[i];
   }
  
 for (size_t i = 0; i < 4; i++)
   {
      rtU.actspeed[i] = rtY.desspeed[i]*0.8;
   }
 rtU.dist2goal = 30.0;  // meters
 rtU.steerang  = 1.0;


 
  while (1) {
/**
 *  pb_encoding_t enc;

    result_t res = DBMMsgEncode(10.0f, 30.0f, 2.5f, &enc);

    if (res != RESULT_OK)
    {
    //error msg
    free(enc.data);
    }
    //Ethsend(enc.data, enc.length);
    free(enc.data);
 */


    rtU.dist2goal = 10.0; // meters
    rtU.steerang = 30.0;
    /**
    LOGI(TAG, "desspeed[0]   = %f, desspeed[1]   = %f, desspeed[2]   = %f, desspeed[3]   = %f, desspeed[4]   = %f, desspeed[5]   = %f\n", rtY.desspeed[0], rtY.desspeed[1], rtY.desspeed[2], rtY.desspeed[3], rtY.desspeed[4], rtY.desspeed[5]);
    LOGI(TAG, "controlb[0]   = %f, controlb[1]   = %f, controlb[2]   = %f, controlb[3]   = %f, controlb[4]   = %f, controlb[5]   = %f\n", rtY.controlb[0], rtY.controlb[1], rtY.controlb[2], rtY.controlb[3], rtY.controlb[4], rtY.controlb[5]);
    LOGI(TAG, "desang[0]     = %f, desang[1]     = %f, desang[2]     = %f, desang[3]     = %f\n", rtY.desang[0], rtY.desang[1], rtY.desang[2], rtY.desang[3]);
    LOGI(TAG, "pwnenable[0]  = %f, pwnenable[1]  = %f, pwnenable[2]  = %f, pwnenable[3]  = %f\n", rtY.pwnenable[0], rtY.pwnenable[1], rtY.pwnenable[2], rtY.pwnenable[3]);
    LOGI(TAG, "pwmrev[0]     = %f, pwmrev[1]     = %f, pwmrev[2]     = %f, pwmrev[3]     = %f\n", rtY.pwmrev[0], rtY.pwmrev[1], rtY.pwmrev[2], rtY.pwmrev[3]);
  
     */
    //BSP_LED_Toggle(LED_GREEN);
    //BSP_LED_Toggle(LED_BLUE);
    //BSP_LED_Toggle(LED_RED);
    //LOGI(TAG, "%d + %d = %d", 5, 2, add(5, 2));

    LOGI(TAG, "This is the driving board");
    osDelay(1000);
    
  }
}

void PwmTask(void *argument){
   const uint32_t period_ms = 1;

   for(;;)
   {
       control_step();// from control.c
       //LOGI(TAG, "control step occured");
       set_bldc_pwm();//this might also be done somewhere else im not sure
       //set_stepper_pwm();
       osDelay(period_ms); //fixed 1ms loop
   }
}

void DrivingSensorTask(void *argument){
    osDelay(999999);
}

#endif //! PIO_UNIT_TESTS
