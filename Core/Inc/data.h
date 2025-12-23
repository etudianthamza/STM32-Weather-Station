/*
 * data.h
 *
 *  Created on: Dec 17, 2025
 *      Author: vdela
 */

#ifndef INC_DATA_H_
#define INC_DATA_H_

#include <stdint.h>
#include "rtc.h"

typedef struct
{
    float temperature;      // °C
    float humidite;         // %
    float pression;         // hPa
    float vent_vitesse;     // m/s
    float vent_direction;   // degrés (0–360)
    float pluie;            // mm

    uint16_t annee;         // ex: 2025
	uint8_t  mois;          // 1–12
	uint8_t  jour;          // 1–31
	uint8_t  heure;         // 0–23
	uint8_t  minute;        // 0–59
	uint8_t  seconde;       // 0–59
} meteo_data_t;

extern volatile meteo_data_t current_data;
extern volatile float vitesse_vent;
void meteo_clear_buffer(volatile meteo_data_t *buffer, uint16_t size);
void meteo_append(volatile meteo_data_t *buffer, uint16_t size, const volatile meteo_data_t *new_data);
void create_data();

#endif /* INC_DATA_H_ */
