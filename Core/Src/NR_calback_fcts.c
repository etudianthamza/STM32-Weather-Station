/*
 * NR_calback_fcts.c
 *
 *  Created on: Dec 12, 2025
 *      Author: hamza
 */


#include "stm32f7xx_hal.h"
#include "tim.h"

extern volatile uint8_t Flag_Tim6;
extern volatile uint8_t Flag_Tim2;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        Flag_Tim6 = 1;
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        Flag_Tim2 = 1;
    }
}
