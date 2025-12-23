/*
 * NR_calback_fcts.c
 *
 *  Created on: Dec 12, 2025
 *      Author: hamza
 */

#include "stm32f7xx_hal.h"
#include "Lecture_pluviometre.h"
#include "tim.h"
#include "main.h"

extern volatile int Flag_Tim6;
extern volatile int Flag_Tim2;
extern volatile int Flag_Tim1;
extern volatile int Flag_Tim3;
extern volatile int Flag_Tim7;
extern volatile uint8_t Flag_Touch;
extern volatile int idle_time;


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        Flag_Tim6 = 1;
    }
    if (htim->Instance == TIM7) {
        Flag_Tim7 = 1;
    }
    if (htim->Instance == TIM3) {
        Flag_Tim3 = 1;
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        // Pluviomètre
        Pluviometre_Capture_Handler();
        Flag_Tim2 = 1;
    }
    if (htim->Instance == TIM1 && htim -> Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        Flag_Tim1 = 1;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_11 || GPIO_Pin == GPIO_PIN_13)
    {
    	Flag_Touch = 1;
    }
}
