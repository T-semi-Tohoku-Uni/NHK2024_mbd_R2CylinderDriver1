/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "R2CANIDList.h"
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
#define UP 1
#define DOWN 2
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
FDCAN_HandleTypeDef hfdcan1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
GPIO_TypeDef *GPIOs[8] = {CYL1A_GPIO_Port, CYL1B_GPIO_Port, CYL2A_GPIO_Port, CYL2B_GPIO_Port, CYL3A_GPIO_Port, CYL3B_GPIO_Port, CYL4A_GPIO_Port, CYL4B_GPIO_Port};
uint16_t GPIOPins[8] = {CYL1A_Pin, CYL1B_Pin, CYL2A_Pin, CYL2B_Pin, CYL3A_Pin, CYL3B_Pin, CYL4A_Pin, CYL4B_Pin};

FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[64] = {};

uint8_t flag = 0;
uint8_t status = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_FDCAN1_Init(void);
/* USER CODE BEGIN PFP */
void USR_ArmUp(void);
void USR_ArmDown(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void USR_ArmDown(void){
	HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CYL1B_GPIO_Port, CYL1B_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CYL2B_GPIO_Port, CYL2B_Pin, GPIO_PIN_RESET);

	//free fall
	HAL_Delay(100);
	HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_RESET);

	//brake
	HAL_Delay(200);
	HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL1B_GPIO_Port, CYL1B_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL2B_GPIO_Port, CYL2B_Pin, GPIO_PIN_SET);

	HAL_Delay(80);
	//static
	HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL1B_GPIO_Port, CYL1B_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL2B_GPIO_Port, CYL2B_Pin, GPIO_PIN_RESET);

}

void USR_ArmUp(void){
	HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL1B_GPIO_Port, CYL1B_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL2B_GPIO_Port, CYL2B_Pin, GPIO_PIN_SET);

	HAL_Delay(200);
	for(uint8_t i = 0; i<3; i++){
		HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CYL1B_GPIO_Port, CYL1B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CYL2B_GPIO_Port, CYL2B_Pin, GPIO_PIN_SET);

		HAL_Delay(60);

		HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CYL1B_GPIO_Port, CYL1B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CYL2B_GPIO_Port, CYL2B_Pin, GPIO_PIN_RESET);
		HAL_Delay(60);

	}
	HAL_GPIO_WritePin(CYL1A_GPIO_Port, CYL1A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL1B_GPIO_Port, CYL1B_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CYL2A_GPIO_Port, CYL2A_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CYL2B_GPIO_Port, CYL2B_Pin, GPIO_PIN_SET);

}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs){
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
		/* Retrieve Rx messages from RX FIFO0 */
		if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
			Error_Handler();
		}

		if(RxHeader.Identifier == CANID_ARM){
			if(RxData[0] == 0){
				flag = UP;
			}
			else if(RxData[0] == 1){
				flag = DOWN;
			}
		}

	}
}

int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2,(uint8_t *)ptr,len,10);
    return len;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	setbuf(stdout, NULL);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  FDCAN_FilterTypeDef sFilterConfig;
	sFilterConfig.IdType = FDCAN_STANDARD_ID;
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_MASK;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = CANID_ARM;
	sFilterConfig.FilterID2 = 0x7FF;




  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
		Error_Handler();
	}

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(status != flag){
		  if(flag == UP){
			  USR_ArmUp();
			  printf("Arm Up\r\n");
			  flag = 0;
			  status = UP;
		  }
		  else if(flag == DOWN){
			  USR_ArmDown();
			  printf("Arm Down\r\n");
			  flag = 0;
			  status = DOWN;
		  }
	  }
	  HAL_Delay(10);
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 4;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 16;
  hfdcan1.Init.NominalTimeSeg2 = 3;
  hfdcan1.Init.DataPrescaler = 2;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 16;
  hfdcan1.Init.DataTimeSeg2 = 3;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */



  /* USER CODE END FDCAN1_Init 2 */

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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CYL1A_Pin|CYL1B_Pin|CYL2A_Pin|CYL2B_Pin
                          |CYL3A_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CYL3B_Pin|CYL4A_Pin|CYL4B_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : CYL1A_Pin CYL1B_Pin CYL2A_Pin CYL2B_Pin
                           CYL3A_Pin */
  GPIO_InitStruct.Pin = CYL1A_Pin|CYL1B_Pin|CYL2A_Pin|CYL2B_Pin
                          |CYL3A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CYL3B_Pin CYL4A_Pin CYL4B_Pin LD2_Pin */
  GPIO_InitStruct.Pin = CYL3B_Pin|CYL4A_Pin|CYL4B_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
