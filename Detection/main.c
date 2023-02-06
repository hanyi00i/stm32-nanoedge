/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
#include "NanoEdgeAI.h"
#include "knowledge.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIS3DH_G_CHIP_ADDR (0x18 << 1)  //SA0(=SD0 pin) = Ground
#define LIS3DH_V_CHIP_ADDR (0x19 << 1)	//SA0(=SD0 pin) = Vdd
#define LIS3DH_WHO_AM_I	0x0f
#define LIS3DH_CTRL_REG1 0x20
#define LIS3DH_CTRL_REG4 0x23
#define LIS3DH_OUT_X_L 0x28
#define BUFFER_SIZE     512
#define NB_AXES         3
#define CLASS_NUMBER	3
//uint16_t input_user_buffer[BUFFER_SIZE * NB_AXES]; // Buffer of input values
float output_class_buffer[CLASS_NUMBER]; // Buffer of class probabilities
const char *id2class[CLASS_NUMBER + 1] = { // Buffer for mapping class id to class name
	"unknown",
	"stop",
	"speed_1",
	"speed_3",
};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t buf[32];
uint8_t buff[64];
float x = 1;
float y = 2;
float z = 3;
float acc_buffer[BUFFER_SIZE * NB_AXES];
uint16_t class_number;
uint16_t id_class = 0; // Point to id class (see argument of neai_classification fct)

void fill_buffer(uint16_t acc_buffer[])
{
	/* USER BEGIN */
	  buf[0] = LIS3DH_OUT_X_L | 0x80;

	   for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
	      if (HAL_I2C_IsDeviceReady(&hi2c1, LIS3DH_V_CHIP_ADDR, 3, 100) == 0) { // New data is available
	    	  buf[0] = LIS3DH_OUT_X_L | 0x80;
	    	  HAL_I2C_Master_Transmit(&hi2c1, LIS3DH_V_CHIP_ADDR, buf, 1, HAL_MAX_DELAY);
	    	  HAL_I2C_Master_Receive(&hi2c1, LIS3DH_V_CHIP_ADDR, buf, 6, HAL_MAX_DELAY);
	    	  x = buf[1] << 8 | buf[0];
	    	  y = buf[3] << 8 | buf[2];
	    	  z = buf[5] << 8 | buf[4];
	          acc_buffer[NB_AXES * i] = x;
	          acc_buffer[(NB_AXES * i) + 1] = y;
	          acc_buffer[(NB_AXES * i) + 2] = z;
	      } else {
	    	  i--; // New data not ready
	      }
	   }

	   for (uint16_t isample = 0; isample < NB_AXES * BUFFER_SIZE - 1; isample++) {
		   sprintf((char*)buf, "%d ", acc_buffer[isample]);
		   HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);
	   }
	  sprintf((char*)buf, "%d\n", acc_buffer[NB_AXES * BUFFER_SIZE] -1);
	  HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);

	  HAL_Delay(1);
	/* USER END */
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_StatusTypeDef ret;

  buf[0] = LIS3DH_CTRL_REG1;
  buf[1] = 0x97;
  ret = HAL_I2C_Master_Transmit(&hi2c1, LIS3DH_V_CHIP_ADDR, buf, 2, HAL_MAX_DELAY);
  if ( ret != HAL_OK ) {
	  sprintf((char*)buf,"ErrorTx CTRL_REG1\n");
	  HAL_UART_Transmit(&huart2, buf, strlen(( char*)buf), HAL_MAX_DELAY);
  }

  buf[0] = LIS3DH_CTRL_REG4;
  buf[1] = 0x08;
  ret = HAL_I2C_Master_Transmit(&hi2c1, LIS3DH_V_CHIP_ADDR, buf, 2, HAL_MAX_DELAY);
  if ( ret != HAL_OK ) {
	  sprintf((char*)buf,"ErrorTx CTRL_REG4\n");
	  HAL_UART_Transmit(&huart2, buf, strlen(( char*)buf), HAL_MAX_DELAY);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	   /* we get fresh accelerometer data */
	  buf[0] = LIS3DH_OUT_X_L | 0x80;
	   for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
	      if (HAL_I2C_IsDeviceReady(&hi2c1, LIS3DH_V_CHIP_ADDR, 3, 100) == 0) { // New data is available
	    	  buf[0] = LIS3DH_OUT_X_L | 0x80;
	    	  HAL_I2C_Master_Transmit(&hi2c1, LIS3DH_V_CHIP_ADDR, buf, 1, HAL_MAX_DELAY);
	    	  HAL_I2C_Master_Receive(&hi2c1, LIS3DH_V_CHIP_ADDR, buf, 6, HAL_MAX_DELAY);
	    	  x = buf[1] << 8 | buf[0];
	    	  y = buf[3] << 8 | buf[2];
	    	  z = buf[5] << 8 | buf[4];
	          acc_buffer[NB_AXES * i] = x;
	          acc_buffer[(NB_AXES * i) + 1] = y;
	          acc_buffer[(NB_AXES * i) + 2] = z;
	      } else {
	    	  i--; // New data not ready
	      }
	   }
//	   for (uint16_t isample = 0; isample < NB_AXES * BUFFER_SIZE - 1; isample++) {
//		   sprintf((char*)buf, "%.4f ", acc_buffer[isample]);
//		   HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);
//	   }
//	  sprintf((char*)buf, "%.4f\n", acc_buffer[NB_AXES * BUFFER_SIZE] -1);
//	  HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);

	  /* Initialization ------------------------------------------------------------*/
	  enum neai_state error_code = neai_classification_init(knowledge);
	  if (error_code != NEAI_OK) {
		  /* This happens if the knowledge does not correspond to the library or if the library works into a not supported board. */
		  sprintf((char*)buf, "ErrorTx NanoEdge AI\n");
		  HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);
	  }

	  /* Classification ------------------------------------------------------------*/
	  /* we classify the input signal and get a class id as output */
	  neai_classification(acc_buffer, output_class_buffer, &id_class);
	  /* we print the result to the serial */
	  sprintf((char*)buff, "Class detected: %s (Certainty: %d %%)\n", id2class[id_class], (uint16_t) (output_class_buffer[id_class - 1] * 100));
	  HAL_UART_Transmit(&huart2, buff, strlen((char*)buff), HAL_MAX_DELAY);
	  // the name of the class and the associated probability %

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00707CBB;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LD3_Pin */
  GPIO_InitStruct.Pin = LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

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
