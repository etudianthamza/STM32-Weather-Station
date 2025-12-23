/*
 * data.c
 *
 *  Created on: Dec 17, 2025
 *      Author: vdela
 */
#include "data.h"
#include "rtc.h"
#include "main.h"
#include "Read_Sensors_THP.h"
#include <string.h>
#include "Lecture_anenometer.h"
#include "Lecture_girouette.h"
void create_data() {

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); // OBLIGATOIRE après GetTime

	current_data.annee   = 2000 + sDate.Year;
	current_data.mois    = sDate.Month;
	current_data.jour    = sDate.Date;

	current_data.heure   = sTime.Hours;
	current_data.minute  = sTime.Minutes;
	current_data.seconde = sTime.Seconds;
	current_data.temperature = sensor_data.temperature;      // °C
	current_data.humidite = sensor_data.humidity;         // %
	current_data.pression = sensor_data.pressure;         // hPa
	current_data.vent_vitesse = vitesse_vent;     // m/s
    current_data.vent_direction = current_data.vent_direction;   // degrés (0–360)
	current_data.pluie = 0.0;            // mm
}

void meteo_clear_buffer(meteo_data_t *buffer, uint16_t size)
{
    if (buffer == NULL)
        return;

    for (uint16_t i = 0; i < size; i++)
    {
        buffer[i] = (meteo_data_t){0};
    }
}

void meteo_append(meteo_data_t *buffer, uint16_t size, const meteo_data_t *new_data)
{
    if (size == 0 || buffer == NULL || new_data == NULL)
        return;

    // Décalage vers la gauche
    for (uint16_t i = 0; i < size - 1; i++)
    {
        buffer[i] = buffer[i + 1];
    }

    // Ajout du plus récent à la fin
    buffer[size - 1] = *new_data;
}

