/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (corrected)
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "hts221_reg.h" // driver capteur temperature & humidity
#include "lps22hh_reg.h" // driver capteur pression
#include <stdbool.h> // pour utiliser bool
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* NUCLEO_F401RE: Define communication interface */
#define SENSOR_BUS hi2c1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define BOOT_TIME        5 // ms
#define TX_BUF_DIM       1000
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* HTS221 (humidity & temperature) */
static int16_t data_raw_humidity_hts;
static int16_t data_raw_temperature_hts;
static float_t humidity_perc;
static float_t temperature_degC_hts;
static uint8_t whoamI_hts;

/* LPS22HH (pressure & temperature) */
static uint32_t data_raw_pressure_lps;
static int16_t data_raw_temperature_lps;
static float_t pressure_hPa;
static float_t temperature_degC_lps;
static uint8_t whoamI_lps;
static uint8_t rst_lps;

/* common */
static uint8_t tx_buffer[TX_BUF_DIM];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* platform wrappers prototypes */
static int32_t platform_write_hts(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read_hts(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static int32_t platform_write_lps(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read_lps(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef struct {
  float_t x0;
  float_t y0;
  float_t x1;
  float_t y1;
} lin_t;

float_t linear_interpolation(lin_t *lin, int16_t x)
{
  return ((lin->y1 - lin->y0) * x + ((lin->x1 * lin->y0) -
                                     (lin->x0 * lin->y1)))
         / (lin->x1 - lin->x0);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  /* ================================ HTS221 init ============================ */
  stmdev_ctx_t dev_ctx_hts;
  dev_ctx_hts.write_reg = platform_write_hts;
  dev_ctx_hts.read_reg  = platform_read_hts;
  dev_ctx_hts.mdelay    = platform_delay;
  dev_ctx_hts.handle    = &SENSOR_BUS;

  /* Check device ID */
  whoamI_hts = 0;
  hts221_device_id_get(&dev_ctx_hts, &whoamI_hts);
  if (whoamI_hts != HTS221_ID) {
    /* manage here device not found */
    while (1) { HAL_Delay(1000); }
  }

  /* Read humidity calibration coefficient */
  lin_t lin_hum;
  hts221_hum_adc_point_0_get(&dev_ctx_hts, (uint8_t *)&lin_hum.x0);
  hts221_hum_rh_point_0_get(&dev_ctx_hts, (uint8_t *)&lin_hum.y0);
  hts221_hum_adc_point_1_get(&dev_ctx_hts, (uint8_t *)&lin_hum.x1);
  hts221_hum_rh_point_1_get(&dev_ctx_hts, (uint8_t *)&lin_hum.y1);

  /* Read temperature calibration coefficient */
  lin_t lin_temp;
  hts221_temp_adc_point_0_get(&dev_ctx_hts, (uint8_t *)&lin_temp.x0);
  hts221_temp_deg_point_0_get(&dev_ctx_hts, (uint8_t *)&lin_temp.y0);
  hts221_temp_adc_point_1_get(&dev_ctx_hts, (uint8_t *)&lin_temp.x1);
  hts221_temp_deg_point_1_get(&dev_ctx_hts, (uint8_t *)&lin_temp.y1);

  /* Enable Block Data Update */
  hts221_block_data_update_set(&dev_ctx_hts, PROPERTY_ENABLE);

  /* Set Output Data Rate */
  hts221_data_rate_set(&dev_ctx_hts, HTS221_ODR_1Hz);

  /* Device power on */
  hts221_power_on_set(&dev_ctx_hts, PROPERTY_ENABLE);

  /* ================================ LPS22HH init =========================== */
  stmdev_ctx_t dev_ctx_lps;
  dev_ctx_lps.write_reg = platform_write_lps;
  dev_ctx_lps.read_reg  = platform_read_lps;
  dev_ctx_lps.mdelay    = platform_delay;
  dev_ctx_lps.handle    = &SENSOR_BUS;

  /* Wait sensor boot time */
  platform_delay(BOOT_TIME);

  /* Check device ID */
  whoamI_lps = 0;
  lps22hh_device_id_get(&dev_ctx_lps, &whoamI_lps);
  if (whoamI_lps != LPS22HH_ID) {
    /* manage here device not found */
    while (1) { HAL_Delay(1000); }
  }

  /* Restore default configuration */
  lps22hh_reset_set(&dev_ctx_lps, PROPERTY_ENABLE);

  do {
    lps22hh_reset_get(&dev_ctx_lps, &rst_lps);
  } while (rst_lps);

  /* Enable Block Data Update */
  lps22hh_block_data_update_set(&dev_ctx_lps, PROPERTY_ENABLE);

  /* Set Output Data Rate */
  lps22hh_data_rate_set(&dev_ctx_lps, LPS22HH_1_Hz);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	      static bool got_temp_hts = false;
	      static bool got_hum_hts  = false;
	      static bool got_pres_lps = false;

	      /* HTS221: check status and read humidity & temperature when available */
	      hts221_status_reg_t status1;
	      hts221_status_get(&dev_ctx_hts, &status1);

	      if (status1.h_da) {
	          hts221_humidity_raw_get(&dev_ctx_hts, &data_raw_humidity_hts);
	          humidity_perc = linear_interpolation(&lin_hum, data_raw_humidity_hts);
	          if (humidity_perc < 0) humidity_perc = 0;
	          if (humidity_perc > 100) humidity_perc = 100;
	          got_hum_hts = true;
	      }

	      if (status1.t_da) {
	          hts221_temperature_raw_get(&dev_ctx_hts, &data_raw_temperature_hts);
	          temperature_degC_hts = linear_interpolation(&lin_temp, data_raw_temperature_hts);
	          got_temp_hts = true;
	      }

	      /* LPS22HH: read status register and then pressure when available */
	      lps22hh_status_t status2;
	      lps22hh_read_reg(&dev_ctx_lps, LPS22HH_STATUS, (uint8_t *)&status2, 1);

	      if (status2.p_da) {
	          lps22hh_pressure_raw_get(&dev_ctx_lps, &data_raw_pressure_lps);
	          pressure_hPa = lps22hh_from_lsb_to_hpa(data_raw_pressure_lps);
	          got_pres_lps = true;
	      }

	      /* ===== Quand on a les 3 valeurs, on affiche une seule ligne ===== */
	      if (got_temp_hts && got_hum_hts && got_pres_lps)
	      {
	          printf("Temperature(°C)\t\tHumidity(%%)\t\tPressure(hPa)\r\n");
	          printf("%6.2f\t\t\t%8.2f\t\t%10.2f\r\n",
	                 temperature_degC_hts,
	                 humidity_perc,
	                 pressure_hPa);

	          /* reset pour la prochaine ligne */
	          got_temp_hts = got_hum_hts = got_pres_lps = false;
	      }

	      platform_delay(20); // moins de spam, affichage fluide
  }/* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  /* never reached */
  /* USER CODE END 3 */
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

PUTCHAR_PROTOTYPE
{
  /* write a character to the USART1 and wait until transmitted */
  uint8_t c = (uint8_t)ch;
  HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
  return ch;
}

/* Platform write/read functions for HTS221 (I2C) */
static int32_t platform_write_hts(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) {
    reg_addr |= 0x80; /* auto-increment for multi-byte */
  }
  if (HAL_I2C_Mem_Write(hi2c, HTS221_I2C_ADDRESS, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)bufp, len, 1000) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int32_t platform_read_hts(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) {
    reg_addr |= 0x80; /* auto-increment for multi-byte */
  }
  if (HAL_I2C_Mem_Read(hi2c, HTS221_I2C_ADDRESS, reg_addr, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000) != HAL_OK) {
    return -1;
  }
  return 0;
}

/* Platform write/read functions for LPS22HH (I2C) */
static int32_t platform_write_lps(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) {
    reg_addr |= 0x80; /* auto-increment for multi-byte */
  }
  if (HAL_I2C_Mem_Write(hi2c, LPS22HH_I2C_ADD_H, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)bufp, len, 1000) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int32_t platform_read_lps(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
  I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
  uint8_t reg_addr = reg;
  if (len > 1) {
    reg_addr |= 0x80; /* auto-increment for multi-byte */
  }
  if (HAL_I2C_Mem_Read(hi2c, LPS22HH_I2C_ADD_H, reg_addr, I2C_MEMADD_SIZE_8BIT, bufp, len, 1000) != HAL_OK) {
    return -1;
  }
  return 0;
}

/* Delay implementation */
static void platform_delay(uint32_t ms)
{
  HAL_Delay(ms);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    /* you can blink an LED here */
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* implementation optional */
}
#endif /* USE_FULL_ASSERT */
