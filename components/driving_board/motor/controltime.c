#include "controltime.h"
#include "control.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bldc.h"
#include "logging.h"


static char *TAG = "TIMER";
extern osThreadId_t controlmotortimHandle;


//FreeRTOS task
void ControlGeneralTime(void *argument)
{
   const TickType_t xPeriod = pdMS_TO_TICKS(1); // 1 ms period
   TickType_t xLastWakeTime = xTaskGetTickCount();


   for(;;)
   {
       control_step();// from control.c
       LOGI(TAG, "control step occured");
       set_bldc_pwm();//this might also be done somewhere else im not sure
       //set_stepper_pwm();
       vTaskDelayUntil(&xLastWakeTime, xPeriod); //fixed 1ms loop
   }
}

//set priorities right
//iinside shouldnt tke to much time
//make wrapper does control step and returns control output input
