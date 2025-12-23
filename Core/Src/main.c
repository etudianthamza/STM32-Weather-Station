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
#include "adc.h"
#include "dma2d.h"
#include "i2c.h"
#include "ltdc.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "screen.h"
#include "Read_Sensors_THP.h"
#include "data.h"
#include "stm32746g_discovery_ts.h"
#include "Lecture_anenometer.h"
#include "Lecture_girouette.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile int Flag_Tim6 = 0;
volatile int Flag_Tim7 = 0;
volatile int Flag_Tim2 = 0;
volatile int Flag_Tim1 = 0;
volatile int Flag_Tim3 = 0;
volatile uint8_t Flag_Touch = 0;
volatile uint8_t Flag_Girouette = 0;  //Flag pour la girouette
int current_page = 0;
volatile meteo_data_t current_data;
volatile meteo_data_t min_data[60]; //1 donnée par seconde pour la dernière minute
//on garde une donnée sur 60 dans la 2nd liste pour économiser mémoire
volatile meteo_data_t day_data[1440]; //1 donnée par minute pour les dernières 24h
volatile float vitesse_vent = 0.0;
volatile GirouetteData_t girouette_data;  //  Données girouette
int data_index;
volatile int idle_time;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
void RTC_Init();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  MX_TIM2_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM7_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */
    /* LCD Initialization */

  	  init_screen();
    // Initialisation des capteurs (remplace votre code d'init dans Read_Sensors_THP.c)
    SensorsTHP_Init();
    Anemometer_Init();          // Anémomètre
    Girouette_Init();           // Girouette
    RTC_Init();
    idle_time = 0;

    //Initialisation des données
    current_data = (meteo_data_t){0};
    data_index = 0;
    meteo_clear_buffer(min_data, 60);
    meteo_clear_buffer(day_data, 1440);

    /* Start TIM6 interrupt */
    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_Base_Start_IT(&htim7);
    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    //  Démarrer la girouette (ADC en continu)
        Girouette_Start_Conversion();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {

        if (Flag_Tim3)
        {
            Flag_Tim3 = 0;

            TS_StateTypeDef ts;
            BSP_TS_GetState(&ts);

            if (ts.touchDetected)
            {
                uint16_t x = ts.touchX[0];
                uint16_t y = ts.touchY[0];

                idle_time = 0;
                current_page = handle_touch(x, y, current_page);
            }
            else
            {
                idle_time++;
            }
        }
        else if(Flag_Touch == 1)
        {
        	if (idle_time >= 2000U){ // vérifie est ce qu'on attient la durée de 1 min pour lancer la routine
        	Flag_Touch = 0;
            idle_time = 0;

            __HAL_TIM_SET_COUNTER(&htim3, 0);
            __HAL_TIM_SET_COUNTER(&htim6, 0);
            __HAL_TIM_SET_COUNTER(&htim7, 0);

            HAL_TIM_Base_Start_IT(&htim6);
            HAL_TIM_Base_Start_IT(&htim7);
            HAL_TIM_Base_Start_IT(&htim3);
            HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);

            // Réinitialisez le touch screen
            BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
            // Redessinez l'écran
            init_screen();
            update_screen(current_page);}
        	else // idle_time < durée (60s), on laisse le système dans son mode normal
        		idle_time = 0;
        		Flag_Touch = 0;
        }
        else if (Flag_Tim7)
        {
            update_screen(current_page);
            Flag_Tim7 = 0;
        }
    	else if (Flag_Tim6)
        {
            // Lecture des capteurs température/humidité/pression
        	SensorsTHP_ReadAll();
            printf("Temp: %.2f°C | Hum: %.2f%% | Press: %.2fhPa\r\n",
                   sensor_data.temperature,
                   sensor_data.humidity,
                   sensor_data.pressure);

            // Lecture de la girouette
            Girouette_Read_Direction(&girouette_data);
            // Mettre à jour les données courantes
            current_data.vent_direction = girouette_data.angle_deg;
            printf("Direction de vent: %s (ADC=%u, Angle=%.1f°)\r\nVitesse du vent: %.2fkm/h \r\n",
                   girouette_data.direction_courte,
                   girouette_data.valeur_adc,
                   girouette_data.angle_deg,
				   vitesse_vent);
            Flag_Tim6 = 0;
            // Mise à jour de l'écran
            create_data();
            data_index++;
            if (data_index >= 60)
            {
                data_index = 0;
                // Une fois par minute
                meteo_append(day_data, 1440, &current_data);
            }
            meteo_append(min_data, 60, &current_data);
        }
        else if (Flag_Tim1)
        {
            Lecture_anenometer(&vitesse_vent);
            Flag_Tim1 = 0;
        }
        else
        {
            if(idle_time >= 2000U)  // 60 secondes pour que le système sera en Mode Stop
            {
                // Entrez en mode STOP
                HAL_SuspendTick();
                HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
	 			HAL_ResumeTick();
		        SystemClock_Config();       // reconfig horloge après STOP
            }
            else
            {
            	// Sinon, Entrez en mode STOP
                HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
            }
   	    }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief Fonction printf redirigée vers UART
  */
PUTCHAR_PROTOTYPE
{
  uint8_t c = (uint8_t)ch;
  HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
  return ch;
}

void RTC_Init() {
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0xBEEF)
	{
		RTC_TimeTypeDef sTime = {0};
		RTC_DateTypeDef sDate = {0};

		sTime.Hours   = 12;
		sTime.Minutes = 0;
		sTime.Seconds = 0;

		sDate.Year  = 25;               // 2025
		sDate.Month = RTC_MONTH_JANUARY;
		sDate.Date  = 1;
		sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;

		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0xBEEF);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
