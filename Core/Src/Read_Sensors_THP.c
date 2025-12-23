/*
 * Read_Sensors_THP.c
 *
 *  Created on: Dec 16, 2025
 *      Author: hamza
 */

#include "Read_Sensors_THP.h"

// ==== VARIABLES GLOBALES ====
volatile SensorData_t sensor_data = {0};
stmdev_ctx_t dev_ctx_hts = {0};
stmdev_ctx_t dev_ctx_lps = {0};
#define SENSOR_BUS hi2c1;

// Variables pour calibration HTS221
static lin_t lin_hum = {0};
static lin_t lin_temp = {0};
static int16_t data_raw_humidity_hts = 0;
static int16_t data_raw_temperature_hts = 0;
static uint32_t data_raw_pressure_lps = 0;

#define HTS221_ADDR  (HTS221_I2C_ADDRESS & 0xFE)
#define LPS22HH_ADDR (LPS22HH_I2C_ADD_H & 0xFE)


/* Platform write/read functions for HTS221 (I2C) */
static int32_t platform_write_hts(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) reg_addr |= 0x80;
  return (HAL_I2C_Mem_Write(hi2c, HTS221_ADDR, reg_addr,
                           I2C_MEMADD_SIZE_8BIT, (uint8_t*)bufp, len, 1000) == HAL_OK) ? 0 : -1;
}

static int32_t platform_read_hts(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) reg_addr |= 0x80;
  return (HAL_I2C_Mem_Read(hi2c, HTS221_ADDR, reg_addr,
                          I2C_MEMADD_SIZE_8BIT, bufp, len, 1000) == HAL_OK) ? 0 : -1;
}

/* Platform write/read functions for LPS22HH (I2C) */
static int32_t platform_write_lps(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) reg_addr |= 0x80;
  return (HAL_I2C_Mem_Write(hi2c, LPS22HH_I2C_ADD_H, reg_addr,
                           I2C_MEMADD_SIZE_8BIT, (uint8_t*)bufp, len, 1000) == HAL_OK) ? 0 : -1;
}

static int32_t platform_read_lps(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) reg_addr |= 0x80;
  return (HAL_I2C_Mem_Read(hi2c, LPS22HH_I2C_ADD_H, reg_addr,
                          I2C_MEMADD_SIZE_8BIT, bufp, len, 1000) == HAL_OK) ? 0 : -1;
}

// ==== FONCTION D'INTERPOLATION LINÉAIRE (EXISTANTE) ====
float_t linear_interpolation(lin_t *lin, int16_t x)
{
    return ((lin->y1 - lin->y0) * x + ((lin->x1 * lin->y0) - (lin->x0 * lin->y1))) / (lin->x1 - lin->x0);
}

// ==== INITIALISATION DES CAPTEURS T/H/P ====
void SensorsTHP_Init(void)
{
    HTS221_Init();
    LPS22HH_Init();
}

// == the temperature and the humidity initialisations === //
void HTS221_Init(void)
{
    // Configuration du contexte HTS221
    dev_ctx_hts.write_reg = platform_write_hts;
    dev_ctx_hts.read_reg  = platform_read_hts;
    dev_ctx_hts.handle    = &SENSOR_BUS;
    dev_ctx_hts.mdelay    = NULL;

    // Vérification de l'ID du capteur
    uint8_t whoamI_hts = 0;
    hts221_device_id_get(&dev_ctx_hts, &whoamI_hts);
    if (whoamI_hts != HTS221_ID) {
        Error_Handler();
    }

    // Lecture des paramètres de calibration
    hts221_hum_adc_point_0_get(&dev_ctx_hts, &lin_hum.x0);
    hts221_hum_rh_point_0_get(&dev_ctx_hts, &lin_hum.y0);
    hts221_hum_adc_point_1_get(&dev_ctx_hts, &lin_hum.x1);
    hts221_hum_rh_point_1_get(&dev_ctx_hts, &lin_hum.y1);

    hts221_temp_adc_point_0_get(&dev_ctx_hts, &lin_temp.x0);
    hts221_temp_deg_point_0_get(&dev_ctx_hts, &lin_temp.y0);
    hts221_temp_adc_point_1_get(&dev_ctx_hts, &lin_temp.x1);
    hts221_temp_deg_point_1_get(&dev_ctx_hts, &lin_temp.y1);

    // Activer capteur - PAS DE CONFIGURATION ODR
    hts221_ctrl_reg1_t ctrl_reg1_hts;
    ctrl_reg1_hts.pd = PROPERTY_ENABLE;  // Power ON seulement
    ctrl_reg1_hts.bdu = PROPERTY_ENABLE; // Block data update
    hts221_write_reg(&dev_ctx_hts, HTS221_CTRL_REG1, (uint8_t*)&ctrl_reg1_hts, 1);
}

void LPS22HH_Init(void)
{
    // Configuration du contexte LPS22HH
    dev_ctx_lps.write_reg = platform_write_lps;
    dev_ctx_lps.read_reg  = platform_read_lps;
    dev_ctx_lps.handle    = &SENSOR_BUS;
    dev_ctx_lps.mdelay    = NULL;

    // Vérification de l'ID du capteur
    uint8_t whoamI_lps = 0;
    lps22hh_device_id_get(&dev_ctx_lps, &whoamI_lps);
    if (whoamI_lps != LPS22HH_ID) {
        Error_Handler();
    }

    // Activer capteur - PAS DE CONFIGURATION ODR
    lps22hh_ctrl_reg1_t ctrl_reg1_lps;
    ctrl_reg1_lps.bdu = 1;  // Block data update seulement
    lps22hh_write_reg(&dev_ctx_lps, LPS22HH_CTRL_REG1, (uint8_t*)&ctrl_reg1_lps, 1);
}

// ==== FONCTIONS DE LECTURE (EXISTANTES) ====
void LireCapteursTemp_Hum(volatile float *hum_hts, volatile float *temp_hts)
{
    hts221_status_reg_t status;
    // Lancer mesure one-shot
    uint8_t ctrl = 0;
    hts221_read_reg(&dev_ctx_hts, HTS221_CTRL_REG2, &ctrl, 1);
    ctrl |= 0x01; // One-shot
    hts221_write_reg(&dev_ctx_hts, HTS221_CTRL_REG2, &ctrl, 1);

    // Vérifier si données disponibles
    hts221_status_get(&dev_ctx_hts, &status);

    if (status.h_da)
    {
        hts221_humidity_raw_get(&dev_ctx_hts, &data_raw_humidity_hts);
        *hum_hts = linear_interpolation(&lin_hum, data_raw_humidity_hts);
    }
    else
    {
        *hum_hts = 0.0f;
    }

    if (status.t_da)
    {
        hts221_temperature_raw_get(&dev_ctx_hts, &data_raw_temperature_hts);
        *temp_hts = linear_interpolation(&lin_temp, data_raw_temperature_hts);
    }
    else
    {
        *temp_hts = 0.0f;
    }
}

void LireCapteurPression(volatile float *press_lps)
{
    lps22hh_status_t status;
    uint32_t t0 = HAL_GetTick();

    //Mesure One-shot
    lps22hh_data_rate_set(&dev_ctx_lps, LPS22HH_ONE_SHOOT);

    //Attends donnée pression (p_da=1)  timeout
    do {
        lps22hh_read_reg(&dev_ctx_lps, LPS22HH_STATUS, (uint8_t *)&status, 1);
        if ((HAL_GetTick() - t0) > 50) {     // 50 ms timeout
            *press_lps = 0.0f;
            return;
        }
    } while (!status.p_da);
    lps22hh_pressure_raw_get(&dev_ctx_lps, &data_raw_pressure_lps);
    *press_lps = lps22hh_from_lsb_to_hpa(data_raw_pressure_lps);
}


// ==== FONCTION DE LECTURE TOUT-EN-UN ====
void SensorsTHP_ReadAll(void)
{
    LireCapteursTemp_Hum(&sensor_data.humidity, &sensor_data.temperature);
    LireCapteurPression(&sensor_data.pressure);
}

