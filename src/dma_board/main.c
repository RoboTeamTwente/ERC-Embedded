#include "cubemx_main.h"
#include "dma.h"
#include "gpio.h"
#include "tim.h"

COM_InitTypeDef BspCOMInit;

void SystemClock_Config(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int a = 0;
uint32_t remaining;
uint32_t par;

static uint16_t pwmData[10];
int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();


  //  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  pwmData[0] = 10; pwmData[1] = 20; pwmData[2] = 30;
  pwmData[3] = 40; pwmData[4] = 50; pwmData[5] = 60;
  pwmData[6] = 70; pwmData[7] = 80; pwmData[8] = 90;
  pwmData[9] = 100;
  HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t *)pwmData, 10);
  remaining = __HAL_DMA_GET_COUNTER((&htim2)->hdma[1]);
  while(1){a +=1;
    remaining = __HAL_DMA_GET_COUNTER((&htim2)->hdma[1]);
  }
}
