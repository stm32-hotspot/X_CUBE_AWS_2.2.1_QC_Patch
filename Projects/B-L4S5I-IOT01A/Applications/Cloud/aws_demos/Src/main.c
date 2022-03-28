/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Portions:
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 */

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "string.h"   /* Console output */

/* Demo includes */
#include "aws_demo.h"
#include "aws_dev_mode_key_provisioning.h"
#include "iot_system_init.h"
#include "iot_logging_task.h"
#include "iot_wifi.h"
#include "aws_clientcredential.h"
/* WiFi driver includes. */
#include "es_wifi_io.h"

/* Test accessories includes */
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#include "mbedtls/pem.h"
#include "iot_crypto.h"   /* Crypto heap setup before the DER -> PEM conversion */

/* OTA update watchdog */
#include "iot_config.h"
#include "watchdogkicker.h"
#ifdef SENSOR
#include "sensors_data.h"
#endif

extern void xPortSysTickHandler( void );
extern WIFIReturnCode_t WIFI_GetFirmwareVersion( uint8_t * pucBuffer );
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* The SPI driver polls at a high priority. The logging task's priority must also
 * be high to be not be starved of CPU time. */
#define mainLOGGING_TASK_PRIORITY                       ( configMAX_PRIORITIES - 1 )
#define mainLOGGING_TASK_STACK_SIZE                     ( 100 )
#define mainLOGGING_MESSAGE_QUEUE_LENGTH                ( 15 )
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
static __IO uint8_t button_flags = 0;       /* Counts the button interrupts */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
void MX_SPI3_Init(void);  /* Must be exported to be called from es_wifi_io.c */
static void MX_USART1_UART_Init(void);
static void MX_I2C2_Init(void);


/* USER CODE BEGIN PFP */
static void prvWifiConnect( void );
#ifdef SENSOR
static void prvTelemetryTask( void * pvParameters );
#endif /* SENSOR */
void vDevModePrintDevCertificate( void );
/**
 * @brief Initializes the FreeRTOS heap.
 *
 * Heap_5 is being used to allow non-contiguous regions, therefore the heap
 * needs to be initialized.  See http://www.freertos.org/a00111.html
 */
static void prvInitializeHeap( void );
void vApplicationDaemonTaskStartupHook( void );
void Led_SetState(bool on);
void Led_Blink(int period, int duty, int count);

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
  /* Perform first any hardware initialization that does not require the RTOS to be
   * running.  */
  /* USER CODE END 1 */


  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* Heap_5 is being used to allow non-contiguous memory regions, so the
   * heap must be initialized. */
  prvInitializeHeap();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_SPI3_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  /* USER CODE BEGIN 2 */
#ifdef SENSOR
  init_sensors();
#endif /* SENSOR */

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */

  /* USER CODE BEGIN RTOS_THREADS */
    /* Create tasks that are not dependent on the WiFi being initialized. */
    xLoggingTaskInitialize( mainLOGGING_TASK_STACK_SIZE,
                            mainLOGGING_TASK_PRIORITY,
                            mainLOGGING_MESSAGE_QUEUE_LENGTH );

    xWatchDogKickerTaskInitialize( IOT_THREAD_DEFAULT_STACK_SIZE,
                           IOT_THREAD_DEFAULT_PRIORITY);

#ifdef SENSOR
    xTaskCreate( prvTelemetryTask, "Telemetry", IOT_THREAD_DEFAULT_STACK_SIZE, NULL, IOT_THREAD_DEFAULT_PRIORITY, NULL );
#endif /* SENSOR */

  /* USER CODE END RTOS_THREADS */

  /* Start the scheduler. Initialization that requires the OS to be running,
     * including the WiFi initialization, is performed in the RTOS daemon task
     * startup hook. */
    vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
    return 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config( void )
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
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_I2C2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x307075B1;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  RTC_TimeTypeDef xsTime;
  RTC_DateTypeDef xsDate;
  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  /* Initialize RTC and set the Time and Date. */
  xsTime.Hours = 0x12;
  xsTime.Minutes = 0x0;
  xsTime.Seconds = 0x0;
  xsTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  xsTime.StoreOperation = RTC_STOREOPERATION_RESET;

  if( HAL_RTC_SetTime( &hrtc, &xsTime, RTC_FORMAT_BCD ) != HAL_OK )
  {
    Error_Handler();
  }

  xsDate.WeekDay = RTC_WEEKDAY_FRIDAY;
  xsDate.Month = RTC_MONTH_JANUARY;
  xsDate.Date = 0x03;
  xsDate.Year = 0x20;

  if( HAL_RTC_SetDate( &hrtc, &xsDate, RTC_FORMAT_BCD ) != HAL_OK )
  {
    Error_Handler();
  }
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

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
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, ES_WIFI_RESET_Pin|ES_WIFI_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ES_WIFI_BOOT0_Pin|ES_WIFI_WAKE_UP_Pin|USER_LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPBTLE_RF_SPI3_CSN_GPIO_Port, SPBTLE_RF_SPI3_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ES_WIFI_RESET_Pin */
  GPIO_InitStruct.Pin = ES_WIFI_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ES_WIFI_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : I2C2_SCL_Pin I2C2_SDA_Pin */
  GPIO_InitStruct.Pin = I2C2_SCL_Pin|I2C2_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ES_WIFI_BOOT0_Pin ES_WIFI_WAKE_UP_Pin USER_LED2_Pin */
  GPIO_InitStruct.Pin = ES_WIFI_BOOT0_Pin|ES_WIFI_WAKE_UP_Pin|USER_LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SPBTLE_RF_SPI3_CSN_Pin */
  GPIO_InitStruct.Pin = SPBTLE_RF_SPI3_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPBTLE_RF_SPI3_CSN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ES_WIFI_NSS_Pin */
  GPIO_InitStruct.Pin = ES_WIFI_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(ES_WIFI_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ES_WIFI_DATA_READY_Pin */
  GPIO_InitStruct.Pin = ES_WIFI_DATA_READY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ES_WIFI_DATA_READY_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

void vApplicationIdleHook( void )
{
    static TickType_t xLastPrint = 0;
    TickType_t xTimeNow;
    const TickType_t xPrintFrequency = pdMS_TO_TICKS( 2000 );

    xTimeNow = xTaskGetTickCount();

    if( ( xTimeNow - xLastPrint ) > xPrintFrequency )
    {
        configPRINT( "." );
        xLastPrint = xTimeNow;
    }

    if (button_flags > 0)
    {
      button_flags = 0;
      Led_Blink(100, 50, 5);
    }
}


#ifndef __GNUC__
void * malloc( size_t xSize )
{
    configASSERT( xSize == ~0 );

    return NULL;
}

void * calloc( size_t xNum, size_t xSize )
{
    configASSERT( xSize == ~0 );

    return NULL;
}

void free( void * addr )
{
    configASSERT( true );
}
#endif /* ifndef __GNUC__ */


void vMainUARTPrintString( char * pcString )
{
  const uint32_t ulTimeout = 3000UL;
  HAL_UART_Transmit( &huart1,
                       ( uint8_t * ) pcString,
                       strlen( pcString ),
                       ulTimeout );
}

void vApplicationDaemonTaskStartupHook( void )
{
    WIFIReturnCode_t xWifiStatus;

    /* Turn on the WiFi before key provisioning. This is needed because
     * if we want to use offload SSL, device certificate and key is stored
     * on the WiFi module during key provisioning which requires the WiFi
     * module to be initialized. */
    xWifiStatus = WIFI_On();

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "WiFi module initialized.\r\n" ) );
        /* In STSAFE configuration there is no need for Device Provisioning  */
        /* A simple example to print the device certificate using PKCS#11 interface.
         * This should be replaced by a production ready procedure. */
        vDevModePrintDevCertificate();

        configPRINTF( ( "\r\nShadow Thing Name is %s\r\n", clientcredentialIOT_THING_NAME ) );

        if( SYSTEM_Init() == pdPASS )
        {
            /* Connect to the WiFi before running the demos */
            prvWifiConnect();

            /* Start demos. */
            DEMO_RUNNER_RunDemos();
        }
    }
    else
    {
        configPRINTF( ( "WiFi module failed to initialize.\r\n" ) );

        /* Stop here if we fail to initialize WiFi. */
        configASSERT( xWifiStatus == eWiFiSuccess );
    }
}


static void prvWifiConnect( void )
{
    WIFINetworkParams_t xNetworkParams;
    WIFIReturnCode_t xWifiStatus;
    uint8_t ucIPAddr[ 4 ];
    uint8_t ucWiFiFwVersion[128] = { 0 };

    /* Setup WiFi parameters to connect to access point. */
    xNetworkParams.pcSSID = clientcredentialWIFI_SSID;
    xNetworkParams.ucSSIDLength = sizeof( clientcredentialWIFI_SSID );
    xNetworkParams.pcPassword = clientcredentialWIFI_PASSWORD;
    xNetworkParams.ucPasswordLength = sizeof( clientcredentialWIFI_PASSWORD );
    xNetworkParams.xSecurity = clientcredentialWIFI_SECURITY;
    xNetworkParams.cChannel = 0;

    WIFI_GetFirmwareVersion( ucWiFiFwVersion );
    configPRINTF( ( "WiFi Firmware Version %s.\r\n", ucWiFiFwVersion ) );

    /* Try connecting using provided wifi credentials. */
    xWifiStatus = WIFI_ConnectAP( &( xNetworkParams ) );

    if( xWifiStatus == eWiFiSuccess )
    {
        configPRINTF( ( "WiFi connected to AP %s.\r\n", xNetworkParams.pcSSID ) );

        xWifiStatus = WIFI_GetIP( ucIPAddr );
        if ( eWiFiSuccess == xWifiStatus )
        {
            configPRINTF( ( "IP Address acquired %d.%d.%d.%d\r\n",
                        ucIPAddr[ 0 ], ucIPAddr[ 1 ], ucIPAddr[ 2 ], ucIPAddr[ 3 ] ) );
        }
    }
    else
    {
        /* Connection failed configure softAP to allow user to set wifi credentials. */
        configPRINTF( ( "WiFi failed to connect to AP %s.\r\n", xNetworkParams.pcSSID ) );

        xNetworkParams.pcSSID = wificonfigACCESS_POINT_SSID_PREFIX;
        xNetworkParams.pcPassword = wificonfigACCESS_POINT_PASSKEY;
        xNetworkParams.xSecurity = wificonfigACCESS_POINT_SECURITY;
        xNetworkParams.cChannel = wificonfigACCESS_POINT_CHANNEL;

        configPRINTF( ( "Connect to softAP %s using password %s. \r\n",
                        xNetworkParams.pcSSID, xNetworkParams.pcPassword ) );

        while( WIFI_ConfigureAP( &xNetworkParams ) != eWiFiSuccess )
        {
            configPRINTF( ( "Connect to softAP %s using password %s and configure WiFi. \r\n",
                            xNetworkParams.pcSSID, xNetworkParams.pcPassword ) );
        }

        configPRINTF( ( "WiFi configuration successful. \r\n", xNetworkParams.pcSSID ) );
    }
}


static void prvInitializeHeap( void )
{
    static uint8_t ucHeap1[ configTOTAL_HEAP_SIZE ];

    HeapRegion_t xHeapRegions[] =
    {
        { ( unsigned char * ) ucHeap1, sizeof( ucHeap1 ) },
        { NULL,                                        0 }
    };

    vPortDefineHeapRegions( xHeapRegions );
}




#if ( ( configUSE_TRACE_FACILITY == 1 ) && (INCLUDE_uxTaskGetStackHighWaterMark == 1) )
void print_stacks_highwatermarks( void )
{
  UBaseType_t task_nr =	uxTaskGetNumberOfTasks();
  TaskStatus_t *p_task_status = pvPortMalloc(task_nr * sizeof(TaskStatus_t));
  if (p_task_status != NULL)
  {
    if (task_nr == uxTaskGetSystemState(p_task_status, task_nr, NULL))
    {
      for (int i = 0; i < task_nr; i++)
      {
        vLoggingPrintf("StackHighWaterMark: %s=%lu\n", p_task_status[i].pcTaskName, uxTaskGetStackHighWaterMark(p_task_status[i].xHandle));
      }
    }
    vPortFree(p_task_status);
  }
}
#endif

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    case (USER_BUTTON_Pin):
    {
      button_flags++;
      break;
    }
    case( ES_WIFI_DATA_READY_Pin ):
    {
      SPI_WIFI_ISR();
      break;
    }
    default:
    {
      break;
    }
  }
}

void Led_SetState(bool on)
{
  HAL_GPIO_WritePin(USER_LED2_GPIO_Port, USER_LED2_Pin, (on == true) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Blink LED for 'count' cycles of 'period' period and 'duty' ON duration.
 * duty < 0 tells to start with an OFF state.
 */
void Led_Blink(int period, int duty, int count)
{
  if ( (duty > 0) && (period >= duty) )
  {
    /*  Shape:   ____
                  on |_off__ */
    do
    {
      Led_SetState(true);
      HAL_Delay(duty);
      Led_SetState(false);
      HAL_Delay(period - duty);
    } while (count--);
  }
  if ( (duty < 0) && (period >= -duty) )
  {
    /*  Shape:         ____
                __off_| on   */
    do
    {
      Led_SetState(false);
      HAL_Delay(period + duty);
      Led_SetState(true);
      HAL_Delay(-duty);
    } while (count--);
  }
}


/*-----------------------------------------------------------*/
#define PEM_BEGIN_CRT           "-----BEGIN CERTIFICATE-----\n"
#define PEM_END_CRT             "-----END CERTIFICATE-----\n"

void vDevModePrintDevCertificate( void )
{
  CK_RV xResult = CKR_OK;
  CK_FUNCTION_LIST_PTR pxFunctionList = NULL;
  CK_SESSION_HANDLE xSession = 0;
  CK_ATTRIBUTE xTemplate[ 2 ];
  CK_OBJECT_HANDLE xCertObj = 0;
  CK_BYTE * pxCertificate = NULL;
  CK_BYTE pxCertificatePem[1000];
  size_t olen;

  xResult = C_GetFunctionList( &pxFunctionList );

  /* Initialize the PKCS Module */
  if( xResult == CKR_OK )
  {
    xResult = xInitializePkcs11Token();
  }

  if( xResult == CKR_OK )
  {
    xResult = xInitializePkcs11Session( &xSession );
  }

  if( xResult == CKR_OK )
  {
    /* Get the handle of the device client certificate. */
    xResult = xFindObjectWithLabelAndClass( xSession,
        pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
        CKO_CERTIFICATE,
        &xCertObj );
  }

  if( 0 == xResult )
  {
    /* Query the device certificate size. */
    xTemplate[ 0 ].type = CKA_VALUE;
    xTemplate[ 0 ].ulValueLen = 0;
    xTemplate[ 0 ].pValue = NULL;
    xResult = ( BaseType_t ) pxFunctionList->C_GetAttributeValue( xSession,
        xCertObj,
        xTemplate,
        1 );
  }

  if( 0 == xResult )
  {
    /* Create a buffer for the certificate. */
    pxCertificate = ( CK_BYTE_PTR ) pvPortMalloc( xTemplate[ 0 ].ulValueLen ); /*lint !e9079 Allow casting void* to other types. */

    if( NULL == pxCertificate )
    {
      xResult = ( BaseType_t ) CKR_HOST_MEMORY;
    }
  }

  if( 0 == xResult )
  {
    /* Export the certificate. */
    xTemplate[ 0 ].pValue = pxCertificate;
    xResult = ( BaseType_t ) pxFunctionList->C_GetAttributeValue( xSession,
        xCertObj,
        xTemplate,
        1 );
    configPRINTF( ( "Device Certificate (DER), size = %d \r\n", xTemplate[ 0 ].ulValueLen) );
  }

  /* Convert to PEM. */
  if( 0 == xResult )
  {
    CRYPTO_ConfigureHeap();
    xResult = mbedtls_pem_write_buffer( PEM_BEGIN_CRT, PEM_END_CRT, pxCertificate,
        xTemplate[ 0 ].ulValueLen, pxCertificatePem,
        sizeof(pxCertificatePem), &olen );
  }

  /* Print the certificate. */
  if( 0 == xResult )
  {
    configPRINTF( ( "Device Certificate (PEM), size = %d \r\n%s", olen, pxCertificatePem ) );
  }

  if( xResult == CKR_OK )
  {
    if( pxCertificate != NULL  )
    {
      vPortFree( pxCertificate );
    }

    pxFunctionList->C_CloseSession( xSession );
  }
}

#ifdef SENSOR
static void prvTelemetryTask( void * pvParameters )
{
  char tbuf[500] = { 0 };
  for( ; ; )
  {
    PrepareSensorsData(tbuf, sizeof(tbuf), NULL);
    vLoggingPrintf("%s\n", tbuf);
    vTaskDelay(5000); // refresh every 5 s
  }
}
#endif /* SENSOR */
/* USER CODE END 4 */
/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* Prevent the tick to call FreeRTOS handler before it is started. */
  if (htim->Instance == TIM6) {
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
      xPortSysTickHandler( );
    }
  }

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  while( 1 )
  {
    Led_Blink(200, 100, 1);
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
void assert_failed(char *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
