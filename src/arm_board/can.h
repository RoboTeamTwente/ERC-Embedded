#ifndef CAN_H
#define CAN_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

void CAN_ConfigRx_AllStandard(void);
void CAN_LogStatus(FDCAN_HandleTypeDef *hfdcan);
#endif // !CAN_H
