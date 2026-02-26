/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ABS(a)  (a < 0 ? -a : a)
#define PRINTVEC(v,a,b)                         \
do {                                              \
    int _i; \
    int _longitud = sizeof(v) / sizeof(v[0]);     \                                   
    if ((a) <= (b) && (a) >= 0 && (b) < _longitud) {   \
        for (_i = (a); _i <= (b); _i++) {         \
            printf("Abs en posición %d: %d\n",    \
                   _i, ABS((v)[_i]));             \
        }                                         \
    } else {                                      \
        printf("Índices fuera de rango\n");       \
    }                                             \
} while(0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int _write(int file, char *ptr, int len){
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++){
   	//ITM_SendChar( *ptr++ );
   	HAL_UART_Transmit(&huart2, (uint8_t*)ptr++,1,1000);
    }
    return len;
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*  punt1.c */

void ejer1(void){
 int v[3]={0,0,0}, *p=(int *) 1;

  printf ("\r\n\r\n\r\nEjer1\r\n\r\n");

  printf("1   	 v= %p ; Valores v: %d %d %d\r\n",v, v[0], v[1], v[2]);
  printf("2   	 p= %p ; Valores p: %x %x %x\r\n",p, p[0], p[1], p[2]);
  v[0]=9; v[1]=8; v[2]=7; v[3]=6;
  printf("3   	 v= %p ; Valores v: %d %d %d\r\n",v, v[0], v[1], v[2]);
  p=v;
  printf("4   	 p= %p ; Valores p: %d %d %d\r\n",p, p[0], p[1], p[2]);
}

void ejer2(void){
 int v[3]={0,0,0}, *p=(int *) 1;

  printf ("\r\n\r\n\r\nEjer2\r\n\r\n");

  printf("1   	 v= %p ; Valores v: %d %d %d\r\n",v, v[0], v[1], v[2]);
  printf("2   	 p= %p ; Valores p: %x %x %x\r\n",p, p[0], p[1], p[2]);
  v[0]=9; v[1]=8; v[2]=7; v[3]=6;
  printf("3   	 v= %p ; Valores v: %d %d %d\r\n",v, *v, *(v+1), *(v+2));
  p=v;
  printf("4   	 p= %p ; Valores p: %d %d %d\r\n",p, *p, *(p+1), *(p+2));
}

void ejer3(void){
 int v[3]={0,0,0}, *p=(int *) 1;
 int v1= 10, v2 = 15, v3 = 20;

  printf ("\r\n\r\n\r\nEjer3\r\n\r\n");
  p = &v1;
  printf("v1   	 p= %p ; Valores p: %d %d %d\r\n",p, p[0], p[1], p[2]);
  p = &v2;
  printf("v2   	 p= %p ; Valores p: %d %d %d\r\n",p, *p, *(p+1), *(p+2));
  p = &v3;
  printf("v3   	 p= %p ; Valores p: %d %d %d\r\n",p, *p, *(p+1), *(p+2));
  v[0]=9; v[1]=8; v[2]=7; v[3]=6;
  printf("--3.2--\n");

  p = &v[1];

  printf("v   	 p= %p ; Segundo elemento de p: %d\r\n",p, *p);
}


void ejer4(void){
  int v[3]={0,0,0}, *p=(int *) 1;
  int **ppi = &p;
  *ppi = v;
  int v1= 10, v2 = 15, v3 = 20;
   v[0]=9; v[1]=8; v[2]=7; v[3]=6;

  printf ("\r\n\r\n\r\nEjer4\r\n\r\n");
  printf("p   	 p= %p ; Valores p: %d %d %d\r\n",p, p[0], p[1], p[2]);
}

int suma1(int x1, int x2);
void suma2(int x1, int x2, int *x3);

void ejer5(void){
  int (*p_oper1)(int x1, int x2);
  void (*p_oper2)(int x1, int x2, int *x3);
  int a=6, b=2, c, d;
  printf ("\r\r\n\r\r\nEjer5\r\r\n\r\r\n");
  p_oper1 = suma1;
  c = p_oper1(a, b);
  printf("a + b = %d\r\r\n",c);
  p_oper2 = suma2;
  p_oper2(a, b, &d); printf("a + b = %d\r\r\n",d);

  printf("\n--- FUNCIONES (FLASH) ---\n");
  printf("p_oper1 = %p\n", (void*)p_oper1);
  printf("p_oper2 = %p\n", (void*)p_oper2);
  printf("suma1   = %p\n", (void*)suma1);
  printf("suma2   = %p\n", (void*)suma2);

  printf("\n--- VARIABLES LOCALES (STACK - SRAM) ---\n");
  printf("&a        = %p\n", (void*)&a);
  printf("&b        = %p\n", (void*)&b);
  printf("&c        = %p\n", (void*)&c);
  printf("&d        = %p\n", (void*)&d);

  printf("\n--- PUNTEROS (también en STACK) ---\n");
  printf("&p_oper1  = %p\n", (void*)&p_oper1);
  printf("&p_oper2  = %p\n\n", (void*)&p_oper2);



} /* end main */

void ejer5_4() {
      int a = -10;
    int b = 5;

    printf("ABS(%d) = %d\n", a, ABS(a));
    printf("ABS(%d) = %d\n", b, ABS(b));
}



void ejer5_5() {
  int v[] = { -3, 5, -8, 2, -1, 7 };

  PRINTVEC(v, 0, 5);
  PRINTVEC(v, 1, 3);
}




int suma1(int x1, int x2){
  return (x1+x2);
} /* end suma1 */

void suma2(int x1, int x2, int *x3){
  *x3= x1 + x2;
} /* end suma2 */

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  ejer1();

  ejer2();

  ejer3();

  ejer4();

  ejer5();

  ejer5_4();

  ejer5_5();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
