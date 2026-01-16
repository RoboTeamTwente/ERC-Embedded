
#include "controltime.h"
#include "control.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bldc.h"


//static char *TAG = "MAIN";

//FreeRTOS task
void ControlGeneralTime(void *argument)
{
    const TickType_t xPeriod = pdMS_TO_TICKS(1); // 1 ms period
    TickType_t xLastWakeTime = osKernelGetTickCount();

    for(;;)
    {
        control_step();// from control.c
        //LOGI(TAG, "control step occured");
        set_bldc_pwm();//this might also be done somewhere else im not sure
        //set_stepper_pwm();
        vTaskDelayUntil(&xLastWakeTime, xPeriod); //fixed 1ms loop
    }
}

