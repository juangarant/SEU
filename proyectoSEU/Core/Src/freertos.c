/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
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
/* USER CODE BEGIN Variables */
extern UART_HandleTypeDef huart2;
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* Fase 6: hook de desbordamiento de pila (configCHECK_FOR_STACK_OVERFLOW) */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    const char *m = "\r\nPANIC: STACK OVERFLOW: ";
    (void)xTask;
    HAL_UART_Transmit(&huart2, (uint8_t *)m, strlen(m), 1000);
    HAL_UART_Transmit(&huart2, (uint8_t *)pcTaskName, strlen(pcTaskName), 1000);
    HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, 1000);
    HAL_NVIC_SystemReset();
}
/* USER CODE END Application */

