/*
 * Read_Sensors_THP.h
 *
 *  Created on: Dec 16, 2025
 *      Author: hamza
 */

#ifndef READ_SENSORS_THP_H
#define READ_SENSORS_THP_H

#include "hts221_reg.h"
#include "lps22hh_reg.h"
#include "main.h"

// Structure pour données des capteurs
typedef struct {
    float temperature;
    float humidity;
    float pressure;
} SensorData_t;

// ==== for interpolate ===//
typedef struct {
    float x0, y0;
    float x1, y1;
} lin_t;

// Variables globales
extern volatile SensorData_t sensor_data;
extern stmdev_ctx_t dev_ctx_hts;
extern stmdev_ctx_t dev_ctx_lps;
extern I2C_HandleTypeDef hi2c1;

// Prototypes des fonctions de lecture
void LireCapteursTemp_Hum(volatile float *hum_hts, float volatile *temp_hts);
void LireCapteurPression(volatile float *press_lps);

// Prototype de fonction d'interpolation
float_t linear_interpolation(lin_t *lin, int16_t x);

// Fonctions d'initialisation
void SensorsTHP_Init(void);
void HTS221_Init(void);
void LPS22HH_Init(void);

// Fonctions de lecture directe
void SensorsTHP_ReadAll(void);

#endif
