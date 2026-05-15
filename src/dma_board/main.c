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

static uint32_t pwmData[11];
int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();

  htim2.Instance->CCMR1 |= (1 << 3); //Enable OC1 preload (Bit3)
  htim2.Instance->ARR |= (1000-1);

  //  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  pwmData[0] = 100; pwmData[1] = 200; pwmData[2] = 300;
  pwmData[3] = 400; pwmData[4] = 500; pwmData[5] = 600;
  pwmData[6] = 700; pwmData[7] = 800; pwmData[8] = 900;
  pwmData[9] = 1000; pwmData[10] = 0;
  HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, &pwmData, 11);
  remaining = __HAL_DMA_GET_COUNTER((&htim2)->hdma[1]);
  while(1){
    a +=1;
    remaining = __HAL_DMA_GET_COUNTER((&htim2)->hdma[1]);
  }
}
