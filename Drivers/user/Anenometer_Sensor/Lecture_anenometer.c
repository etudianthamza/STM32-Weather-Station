/*
 * Lecture_anenometer.c
 *
 *  Created on: Dec 13, 2025
 *      Author: hamza
 */

#include "tim.h"

static uint32_t last_capture = 0;
static uint32_t current_capture = 0;
static uint32_t delta_capture = 0;
static uint8_t  first_capture_ready = 0;

void Lecture_anenometer(float *p_wind_speed)
{
    current_capture = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1);

    if (first_capture_ready == 0)
    {
        last_capture = current_capture;
        first_capture_ready = 1;
        *p_wind_speed = 0.0f;
    }
    else
    {
        delta_capture = current_capture - last_capture;

        /* Calcul de la vitesse du vent */
        *p_wind_speed = 1.492f * (1000000.0f / delta_capture) * 3.6f;

        last_capture = current_capture;
    }
}

