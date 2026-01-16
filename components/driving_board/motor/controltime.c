/**
#include "controltime.h"
#include "control.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"


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
        vTaskDelayUntil(&xLastWakeTime, xPeriod); //fixed 1ms loop
    }
}
 */
