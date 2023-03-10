/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
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
#define LED_STACK_SIZE 512
#define TAKE_STACK_SIZE 512
#define GIVE_STACK_SIZE 512
#define LED_PRIORITY 1
#define TAKE_PRIORITY 6
#define GIVE_PRIORITY 5
#define PORT_MAX_DELAY 1000
#define Q_DELAY_LENGTH 10
#define DELAY_SIZE 100*sizeof(uint8_t)
#define LONG_MAX 512

//SemaphoreHandle_t sem1;
SemaphoreHandle_t sem1;
TaskHandle_t hLed;
TaskHandle_t hGive;
TaskHandle_t hTake;
QueueHandle_t q_delay = NULL;
int delay = 100;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

void task_led(void * unused)
{
	while(1)
	{
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
		printf("LED on\r\n");
		vTaskDelay(delay);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
		printf("LED off\r\n");
		vTaskDelay(delay);
	}
}

void task_give(void * unused)
{
	char msg[LONG_MAX];
	while(1)
	{
		printf("Before Notify Give\r\n");
		//xSemaphoreGive(sem1);
		xTaskNotifyGive(hTake);
		printf("After Notify Give\r\n");
		sprintf(msg, "delay %d \r\n",delay);
		xQueueSend(q_delay,(const void *)msg,portMAX_DELAY);

		vTaskDelay(delay);
	}
}

void task_take(void * unused)
{
	char delayBuff[LONG_MAX];
	while(1)
	{
		printf("Before Notify Take\r\n");
		/*if (xSemaphoreTake(sem1,PORT_MAX_DELAY)){
			printf("After Semaphore Take\r\n");}*/
		if (ulTaskNotifyTake(pdTRUE, PORT_MAX_DELAY))
		{
			printf("After Notify Take\r\n");
		}
		else
		{
			NVIC_SystemReset();
		}
		delay += 100;
		xQueueReceive(q_delay, (void *)delayBuff, portMAX_DELAY);
		printf("%s \r\n",delayBuff);
	}
}
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  BaseType_t ret;

  //ret=xTaskCreate(task_led, "LED", LED_STACK_SIZE, NULL, 1, &hLed);
  //configASSERT(ret == pdPASS);

  q_delay = xQueueCreate(Q_DELAY_LENGTH, DELAY_SIZE);
  //sem1=xSemaphoreCreateBinary();
  ret=xTaskCreate(task_give, "GIVE", GIVE_STACK_SIZE, NULL, GIVE_PRIORITY, &hGive);
  configASSERT(ret == pdPASS);
  ret=xTaskCreate(task_take, "TAKE", TAKE_STACK_SIZE, NULL, TAKE_PRIORITY, &hTake);
  configASSERT(ret == pdPASS);


  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
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

#ifdef  USE_FULL_ASSERT
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
