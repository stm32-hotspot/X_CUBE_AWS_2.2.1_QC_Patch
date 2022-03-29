/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file           : main.c
* @brief          : Main program body
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
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
/* Commane/Uncomment the following # define to switch between the 2 different Option Bytes configurartion */
#if defined(X_CUBE_AWS_200)
#define X_CUBE_AWS_REV "X-CUBE-AWS_2.0.0"
#else
#define X_CUBE_AWS_REV "X-CUBE-AWS_2.2.0"
#endif

#define STDIO_UART_HANDLER huart1

#define Flash_size_data_register_address  0x1FFF75E0
#define DBGMCU_IDCODE_ADDRESS   0xE0042000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t Update_OB = 0;

/*Variable used to handle the Options Bytes*/
static FLASH_OBProgramInitTypeDef OptionsBytesStruct;

const uint16_t * const Flash_size_data_register = (uint16_t *)Flash_size_data_register_address;
const uint32_t * const DBGMCU_IDCODE = (uint32_t *)DBGMCU_IDCODE_ADDRESS;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */ 
  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();
  
  /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
  
  /* Clear PEMPTY bit set (as the code is executed from Flash which is not empty) */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PEMPTY) != 0)
  {
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PEMPTY);
  }
  
  HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);
  
  printf("\n\n\rSetting up the Option Bytes for %s\r\n", X_CUBE_AWS_REV);
  printf("BFB2       : %s\r\n", (OptionsBytesStruct.USERConfig & FLASH_OPTR_BFB2    )? "Enabled":"Disabled");
  printf("DBANK      : %s\r\n", (OptionsBytesStruct.USERConfig & FLASH_OPTR_DBANK   )? "Enabled":"Disabled");
  printf("nSWBOOT0   : %s\r\n", (OptionsBytesStruct.USERConfig & FLASH_OPTR_nSWBOOT0)? "Enabled":"Disabled");
  printf("Flash Size : %d\r\n", *Flash_size_data_register);
  printf("IDCODE     : 0x%X\r\n", *DBGMCU_IDCODE);
  
  OptionsBytesStruct.USERType = 0;
  
#if defined(X_CUBE_AWS_200) /* BFB2 = 0,  DBANK =  1, nSWBOOT0 = 0*/
  if((OptionsBytesStruct.USERConfig & FLASH_OPTR_BFB2) != 0)
  {
    OptionsBytesStruct.USERType    |= OB_USER_BFB2;
    OptionsBytesStruct.USERConfig  &= ~FLASH_OPTR_BFB2;  
    Update_OB = 1;
  }
  
  if((OptionsBytesStruct.USERConfig & FLASH_OPTR_DBANK) == 0)
  {
    OptionsBytesStruct.USERType    |= OB_USER_DUALBANK;
    OptionsBytesStruct.USERConfig  |=  FLASH_OPTR_DBANK;
    Update_OB = 1;
  }
  
  if((OptionsBytesStruct.USERConfig & FLASH_OPTR_nSWBOOT0) != 0)
  {
    OptionsBytesStruct.USERType    |= OB_USER_nSWBOOT0;
    OptionsBytesStruct.USERConfig  &= ~FLASH_OPTR_nSWBOOT0;
    Update_OB = 1;
  }
#else /* BFB2 = 1,  DBANK =  0, nSWBOOT0 = 0*/
  if((OptionsBytesStruct.USERConfig & FLASH_OPTR_BFB2) == 0)  
  {
    OptionsBytesStruct.USERType    |= OB_USER_BFB2;
    OptionsBytesStruct.USERConfig  |= FLASH_OPTR_BFB2;
    Update_OB = 1;
  }
  
  if((OptionsBytesStruct.USERConfig & FLASH_OPTR_DBANK) != 0)
  {
    OptionsBytesStruct.USERType    |= OB_USER_DUALBANK;
    OptionsBytesStruct.USERConfig &= ~FLASH_OPTR_DBANK;
    Update_OB = 1;
  }
  
  if((OptionsBytesStruct.USERConfig & FLASH_OPTR_nSWBOOT0) != 0)
  {
    OptionsBytesStruct.USERType    |= OB_USER_nSWBOOT0;
    OptionsBytesStruct.USERConfig &= ~FLASH_OPTR_nSWBOOT0;
    Update_OB = 1;
  }
#endif
  
  if(Update_OB == 1)
  {
    printf("Updating Option Bytes\r\n");
    
    if(HAL_FLASH_Unlock() == HAL_OK)
    {
      /* Unlock option bytes */
      if(HAL_FLASH_OB_Unlock() == HAL_OK) 
      {
        OptionsBytesStruct.OptionType  = OPTIONBYTE_USER;
        
        /* Program selected option byte */
        if(HAL_FLASHEx_OBProgram(&OptionsBytesStruct) != HAL_OK)
        {
          printf("HAL_FLASHEx_OBProgram Error\r\n");
        }
        
        HAL_FLASH_OB_Launch();
        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();
      }
    }
  }
  else
  {
    printf("Option Bytes properly set\r\n");
  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 30;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */
  
  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */
  
  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  
  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
#ifdef __ICCARM__
int __write(int file, char *ptr, int len)
#else
int _write(int file, char *ptr, int len)
#endif
{
  HAL_UART_Transmit(&STDIO_UART_HANDLER,(uint8_t *)ptr, len, 0xFFFFFFFF);
  return len;
}

#ifdef  __ICCARM__ 
int   __read (int file, char *ptr, int len)
#else
int _read(int file, char *ptr, int len)
#endif
{
  int length = 0;
  int ch = 0;
  do
  {
    HAL_UART_Receive(&STDIO_UART_HANDLER, (uint8_t *)&ch, 1, 0xFFFFFFFF);
    *ptr = ch;
    ptr++;
    length++;
  }while((length < len) && (*(ptr-1) != '\r'));
  return length;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
