/*
 * Lecture_anenometer.h
 *
 *  Created on: Dec 13, 2025
 *      Author: hamza
 */

#ifndef LECTURE_ANENOMETER_H
#define LECTURE_ANENOMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void Anemometer_Init(void);
void Anemometer_Capture_Handler(void);
void Lecture_anenometer(float *vitesse_vent);
uint32_t Anemometer_Get_Pulse_Count(void); // pour debug

#ifdef __cplusplus
}
#endif

#endif // LECTURE_ANENOMETER_H
