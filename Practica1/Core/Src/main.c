/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body — Práctica 1 SEU (STM32F411RE)
  *                   ADC + LEDs + Buzzer + Botones
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensor.h"
#include "ldr.h"
#include "ntc.h"
#include "potenciometro.h"
#include "alarma.h"
#include "display_leds.h"
#include "botones.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/** Período de muestreo de sensores y botones (ms) */
#define TASK_MUESTREO_PERIOD_MS      50U
/** Período de actualización del display (ms) */
#define TASK_VISUALIZACION_PERIOD_MS 20U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */
/** Puntero al sensor actualmente seleccionado (ldr o ntc) */
volatile sensor_t *active_sensor = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);

/* USER CODE BEGIN PFP */
static void Task_Muestreo(void);
static void Task_Visualizacion(void);
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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();

  /* USER CODE BEGIN 2 */

  /* Inicializar módulos de aplicación */
  LDR_Init();
  NTC_Init();
  Alarma_Init();
  Display_Clear();

  /* El sensor activo por defecto es el LDR */
  active_sensor = (volatile sensor_t *)&ldr;

    /* Planificación cooperativa simple por tiempos absolutos en ms */
    uint32_t last_muestreo = HAL_GetTick();
    uint32_t last_visual   = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop (no debe llegarse aquí) */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    uint32_t now = HAL_GetTick();

    if ((now - last_muestreo) >= TASK_MUESTREO_PERIOD_MS)
    {
      last_muestreo = now;
      Task_Muestreo();
    }

    if ((now - last_visual) >= TASK_VISUALIZACION_PERIOD_MS)
    {
      last_visual = now;
      Task_Visualizacion();
    }
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Rutina de muestreo (~50 ms).
 *        Lee los 3 ADC, detecta botones, gestiona alarma y rearme.
 */
static void Task_Muestreo(void)
{
  /* -- Leer sensores ADC -- */
  LDR_Read();
  NTC_Read();

  /* -- Actualizar nivel de alarma según el potenciómetro -- */
  POT_UpdateAlarmLevel(active_sensor);

  /* -- Botón izquierdo: cambiar sensor seleccionado -- */
  if (BtL_Pressed())
    {
    if (active_sensor == (volatile sensor_t *)&ldr)
        {
      active_sensor = (volatile sensor_t *)&ntc;
        }
    else
        {
      active_sensor = (volatile sensor_t *)&ldr;
        }
    /* Al cambiar sensor, actualizar el nivel de alarma inmediatamente */
    POT_UpdateAlarmLevel(active_sensor);
    }

  /* -- Botón derecho: apagar buzzer -- */
  if (BtR_Pressed())
  {
    Alarma_Off();
  }

  /* -- Comprobar condición de alarma -- */
  Alarma_Check(active_sensor);
}

/**
 * @brief Rutina de visualización (~20 ms).
 *        Actualiza los 8 LEDs con el valor del sensor activo
 *        y gestiona el parpadeo del LED de nivel de alarma.
 */
static void Task_Visualizacion(void)
{
  Display_Update(active_sensor);
}

/* USER CODE END 4 */

/**
  * @brief System Clock Configuration
  *        HSI → PLL → 84 MHz SYSCLK, APB1 = 42 MHz, APB2 = 84 MHz
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = 16;
  RCC_OscInitStruct.PLL.PLLN            = 336;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;   /* 336/4 = 84 MHz */
  RCC_OscInitStruct.PLL.PLLQ            = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;    /* HCLK  = 84 MHz */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;      /* APB1  = 42 MHz */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;      /* APB2  = 84 MHz */

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  *        Modo: single-channel, software trigger, 12 bits, right-aligned.
  *        Los canales se reconfiguran dinámicamente en ADC_Read_Channel().
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = DISABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Configuración inicial del canal — se sobreescribirá en cada lectura */
  sConfig.Channel      = ADC_CHANNEL_0;
  sConfig.Rank         = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Configure GPIO pin Output Level — LEDs y Buzzer apagados */
  HAL_GPIO_WritePin(GPIOA, LED8_Pin | LED6_Pin | Buzzer_Pin | LED3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, LED7_Pin | LED2_Pin | LED5_Pin | LED1_Pin | LED4_Pin, GPIO_PIN_RESET);

  /* B1 (PC13): interrupción por flanco de bajada */
  GPIO_InitStruct.Pin  = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /* USART2_TX (PA2): función alternativa */
  GPIO_InitStruct.Pin       = USART_TX_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(USART_TX_GPIO_Port, &GPIO_InitStruct);

  /* Salidas en GPIOA: LED8, LED6, Buzzer, LED3 */
  GPIO_InitStruct.Pin   = LED8_Pin | LED6_Pin | Buzzer_Pin | LED3_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Salidas en GPIOB: LED7, LED2, LED5, LED1, LED4 */
  GPIO_InitStruct.Pin   = LED7_Pin | LED2_Pin | LED5_Pin | LED1_Pin | LED4_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* BtL (PC7): entrada con pull-up */
  GPIO_InitStruct.Pin  = BtL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BtL_GPIO_Port, &GPIO_InitStruct);

  /* BtR (PB6): entrada con pull-up */
  GPIO_InitStruct.Pin  = BtR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BtR_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
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
  (void)file;
  (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
