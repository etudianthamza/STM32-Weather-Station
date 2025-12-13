/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "hts221_reg.h" // header for hts221
#include "lps22hh_reg.h"  // header for lps22hh
#include <stdbool.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SENSOR_BUS hi2c1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

static stmdev_ctx_t dev_ctx_hts;
static stmdev_ctx_t dev_ctx_lps;

static int16_t data_raw_humidity_hts;
static int16_t data_raw_temperature_hts;
static uint32_t data_raw_pressure_lps;

volatile uint8_t Flag_Tim6 = 0;
float humidity, temperature, pressure;

typedef struct {
    float x0, y0;
    float x1, y1;
} lin_t;
static lin_t lin_hum, lin_temp;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

void LireCapteursTemp_Hum(float *hum_hts, float *temp_hts);
void LireCapteurPression(float *press_lps);

float_t linear_interpolation(lin_t *lin, int16_t x);

static int32_t platform_write_hts(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read_hts(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t platform_write_lps(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read_lps(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

/* USER CODE END PFP */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_TIM6_Init();

    /* ===== HTS221 init ===== */
    dev_ctx_hts.write_reg = platform_write_hts;
    dev_ctx_hts.read_reg  = platform_read_hts;
    dev_ctx_hts.handle    = &SENSOR_BUS;
    dev_ctx_hts.mdelay    = NULL;

    uint8_t whoamI_hts;
    hts221_device_id_get(&dev_ctx_hts, &whoamI_hts);
    if (whoamI_hts != HTS221_ID) while(1);

    // Lire calibration
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

    /* ===== LPS22HH init ===== */
    dev_ctx_lps.write_reg = platform_write_lps;
    dev_ctx_lps.read_reg  = platform_read_lps;
    dev_ctx_lps.handle    = &SENSOR_BUS;
    dev_ctx_lps.mdelay    = NULL;

    uint8_t whoamI_lps;
    lps22hh_device_id_get(&dev_ctx_lps, &whoamI_lps);
    if (whoamI_lps != LPS22HH_ID) while(1);

    // Activer capteur - PAS DE CONFIGURATION ODR
    lps22hh_ctrl_reg1_t ctrl_reg1_lps;
    ctrl_reg1_lps.bdu = 1;  // Block data update seulement
    lps22hh_write_reg(&dev_ctx_lps, LPS22HH_CTRL_REG1, (uint8_t*)&ctrl_reg1_lps, 1);

    /* Start TIM6 interrupt */
    HAL_TIM_Base_Start_IT(&htim6);

    /* Infinite loop */
    while (1)
    {
        if (Flag_Tim6)
        {
            LireCapteursTemp_Hum(&humidity, &temperature);
            LireCapteurPression(&pressure);
            printf("T: %.2f°C | H: %.2f%% | P: %.2fhPa\r\n", temperature, humidity, pressure);
            Flag_Tim6 = 0;
        }
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


//====================LireCapteursTemp_Hum================//
void LireCapteursTemp_Hum(float *hum_hts, float *temp_hts)
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

//====================LireCapteurPression================//
void LireCapteurPression(float *press_lps)
{
    lps22hh_status_t status;

    lps22hh_read_reg(&dev_ctx_lps, LPS22HH_STATUS, (uint8_t *)&status, 1);

    if (status.p_da)
    {
        lps22hh_pressure_raw_get(&dev_ctx_lps, &data_raw_pressure_lps);
        *press_lps = lps22hh_from_lsb_to_hpa(data_raw_pressure_lps);
    }
}

/**
  * @brief Interpolation linéaire pour la calibration
  */
float_t linear_interpolation(lin_t *lin, int16_t x)
{
  return ((lin->y1 - lin->y0) * x + ((lin->x1 * lin->y0) -
                                     (lin->x0 * lin->y1)))
         / (lin->x1 - lin->x0);
}

/**
  * @brief Fonction printf redirigée vers UART
  */
PUTCHAR_PROTOTYPE
{
  uint8_t c = (uint8_t)ch;
  HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
  return ch;
}

/* Platform write/read functions for HTS221 (I2C) */
static int32_t platform_write_hts(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) reg_addr |= 0x80;
  return (HAL_I2C_Mem_Write(hi2c, HTS221_I2C_ADDRESS, reg_addr,
                           I2C_MEMADD_SIZE_8BIT, (uint8_t*)bufp, len, 1000) == HAL_OK) ? 0 : -1;
}

static int32_t platform_read_hts(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) reg_addr |= 0x80;
  return (HAL_I2C_Mem_Read(hi2c, HTS221_I2C_ADDRESS, reg_addr,
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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif /* USE_FULL_ASSERT */
