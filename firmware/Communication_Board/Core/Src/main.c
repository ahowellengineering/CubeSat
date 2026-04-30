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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
#include "stm32_hal_legacy.h"
#include <stdint.h>
#include "cc1101.h"
#include "mcp9808.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_irda.h"
#include "stm32f4xx_hal_uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
  uint8_t cc1101_version;
  uint8_t cc1101_partnum;
  uint8_t rssi;
  uint8_t rx_buffer[64];
  uint8_t tx_buffer[64];
} RadioTaskContext_t;

typedef struct { 
  float temperature_c;
  float battery_voltage;
  uint8_t battery_percentage;
} SenseTaskContext_t;

typedef struct __attribute__((packed)) {
  float temperature_c;
  float battery_voltage;
  uint8_t battery_percentage;
} TelemetryPacket_t;

typedef struct {
  uint8_t tx_counter;
} OpticalTaskContext_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

IRDA_HandleTypeDef hirda1;
UART_HandleTypeDef huart6;

/* Definitions for heartBeatTask */
osThreadId_t heartBeatTaskHandle;
const osThreadAttr_t heartBeatTask_attributes = {
  .name = "heartBeatTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for radioTask */
osThreadId_t radioTaskHandle;
const osThreadAttr_t radioTask_attributes = {
  .name = "radioTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for senseTask */
osThreadId_t senseTaskHandle;
const osThreadAttr_t senseTask_attributes = {
  .name = "senseTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for opticalTask */
osThreadId_t opticalTaskHandle;
const osThreadAttr_t opticalTask_attributes = {
  .name = "opticalTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensorDataMutex */
osMutexId_t sensorDataMutexHandle;
const osMutexAttr_t sensorDataMutex_attributes = {
  .name = "sensorDataMutex"
};
/* USER CODE BEGIN PV */
static RadioTaskContext_t radio_ctx = {
  .cc1101_version = 0,
  .cc1101_partnum = 0
};

static SenseTaskContext_t sense_ctx = {
  .temperature_c = 0.0f
};

static TelemetryPacket_t telemetry_packet = {
  .temperature_c = 0.0f,
  .battery_voltage = 0.0f,
  .battery_percentage = 0
};

static OpticalTaskContext_t optical_context = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_IRDA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART6_UART_Init(void);
void StartHeartBeatTask(void *argument);
void StartRadioTask(void *argument);
void StartSenseTask(void *argument);
void StartOpticalTask(void *argument);

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
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART1_IRDA_Init();
  MX_ADC1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of sensorDataMutex */
  sensorDataMutexHandle = osMutexNew(&sensorDataMutex_attributes);

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
  /* creation of heartBeatTask */
  heartBeatTaskHandle = osThreadNew(StartHeartBeatTask, NULL, &heartBeatTask_attributes);

  /* creation of radioTask */
  radioTaskHandle = osThreadNew(StartRadioTask, NULL, &radioTask_attributes);

  /* creation of senseTask */
  senseTaskHandle = osThreadNew(StartSenseTask, NULL, &senseTask_attributes);

  /* creation of opticalTask */
  opticalTaskHandle = osThreadNew(StartOpticalTask, NULL, &opticalTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_IRDA_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  hirda1.Instance = USART1;
  hirda1.Init.BaudRate = 57600;
  hirda1.Init.WordLength = IRDA_WORDLENGTH_8B;
  hirda1.Init.Parity = IRDA_PARITY_NONE;
  hirda1.Init.Mode = IRDA_MODE_TX;
  hirda1.Init.Prescaler = 1;
  hirda1.Init.IrDAMode = IRDA_POWERMODE_NORMAL;
  if (HAL_IRDA_Init(&hirda1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(onboard_led_GPIO_Port, onboard_led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CS_CC1101_Pin|CS_CAM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : onboard_led_Pin */
  GPIO_InitStruct.Pin = onboard_led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(onboard_led_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_CC1101_Pin CS_CAM_Pin */
  GPIO_InitStruct.Pin = CS_CC1101_Pin|CS_CAM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : GD0_CC1101_Pin */
  GPIO_InitStruct.Pin = GD0_CC1101_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GD0_CC1101_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


float Calculate_Battery_Voltage(uint16_t raw_adc) {
    return (float)raw_adc * 0.00117216f; 
}

uint8_t Calculate_Battery_Percentage(float voltage) {
    // 1. Above max threshold (Charging or fully charged)
    if (voltage >= 4.20f) return 100;
    
    // 2. High plateau (4.0V to 4.2V -> 80% to 100%)
    if (voltage >= 4.00f) {
        return 80 + (uint8_t)(20.0f * ((voltage - 4.00f) / 0.20f));
    }
    
    // 3. Middle flat plateau (3.85V to 4.0V -> 50% to 80%)
    if (voltage >= 3.85f) {
        return 50 + (uint8_t)(30.0f * ((voltage - 3.85f) / 0.15f));
    }
    
    // 4. The knee - dropping fast (3.7V to 3.85V -> 20% to 50%)
    if (voltage >= 3.70f) {
        return 20 + (uint8_t)(30.0f * ((voltage - 3.70f) / 0.15f));
    }
    
    // 5. Critical drop-off (3.2V to 3.7V -> 0% to 20%)
    if (voltage > 3.20f) {
        return (uint8_t)(20.0f * ((voltage - 3.20f) / 0.50f));
    }
    
    // 6. Below cut-off (Dead battery)
    return 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GD0_CC1101_Pin)
  {
    osThreadFlagsSet(radioTaskHandle, CC1101_RX_FLAG);
  }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartHeartBeatTask */
/**
  * @brief  Function implementing the heartBeatTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartHeartBeatTask */
void StartHeartBeatTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(onboard_led_GPIO_Port, onboard_led_Pin);
    osDelay(500);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartRadioTask */
/**
* @brief Function implementing the radioTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRadioTask */
void StartRadioTask(void *argument)
{
  /* USER CODE BEGIN StartRadioTask */
  CC1101_t cc1101 = {
    .hspi = &hspi1,
    .csPort = CS_CC1101_GPIO_Port,
    .csPin = CS_CC1101_Pin,
    .gdo0Port = GD0_CC1101_GPIO_Port,
    .gdo0Pin = GD0_CC1101_Pin
  };

  if (!CC1101_Init(&cc1101)) {
    // Initialization failed, handle error (e.g., log, retry, etc.)
    while(1) {
      // Blink error code on LED (e.g., 2 blinks for init failure)
      HAL_GPIO_TogglePin(onboard_led_GPIO_Port, onboard_led_Pin);
      osDelay(200);
      HAL_GPIO_TogglePin(onboard_led_GPIO_Port, onboard_led_Pin);
      osDelay(800);
    }
  }

  radio_ctx.cc1101_version = CC1101_ReadReg(&cc1101, CC1101_VERSION);
  radio_ctx.cc1101_partnum = CC1101_ReadReg(&cc1101, CC1101_PARTNUM);

  // Lower power for Part 15 compliance
  uint8_t pa_table_low_power = 0x12; 
  CC1101_WriteReg(&cc1101, CC1101_PA_TABLE, pa_table_low_power);

  /* Infinite loop */
  uint32_t last_tx_time = osKernelGetTickCount();
  for(;;)
  {

    uint32_t flags = osThreadFlagsWait(CC1101_RX_FLAG, osFlagsWaitAny, 100);

    if ((flags & CC1101_RX_FLAG) == CC1101_RX_FLAG) 
    {
      uint8_t rx_len = CC1101_ReceivePacket(&cc1101, radio_ctx.rx_buffer);

      if (rx_len > 0){
        HAL_UART_Transmit(&huart6, radio_ctx.rx_buffer, rx_len, HAL_MAX_DELAY);
        radio_ctx.rssi = CC1101_GetRSSI(radio_ctx.rx_buffer, rx_len);
      }

      CC1101_Strobe(&cc1101, CC1101_SRX);
    }

    if (osKernelGetTickCount() - last_tx_time >= 10000) 
    {
      if (osMutexAcquire(sensorDataMutexHandle, osWaitForever) == osOK)
      {
        telemetry_packet.temperature_c = sense_ctx.temperature_c;
        telemetry_packet.battery_voltage = sense_ctx.battery_voltage;
        telemetry_packet.battery_percentage = sense_ctx.battery_percentage;
        osMutexRelease(sensorDataMutexHandle);

        CC1101_SendPacket(&cc1101, (uint8_t*)&telemetry_packet, sizeof(telemetry_packet));
        CC1101_Strobe(&cc1101, CC1101_SRX);
        last_tx_time = osKernelGetTickCount();
      } 
    }
  }
  /* USER CODE END StartRadioTask */
}

/* USER CODE BEGIN Header_StartSenseTask */
/**
* @brief Function implementing the senseTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSenseTask */
void StartSenseTask(void *argument)
{
  /* USER CODE BEGIN StartSenseTask */
  uint16_t adc_value = 0;

  if (MCP9808_Init(&hi2c1) != HAL_OK) 
  {
    while(1)
    {
      // Blink error code on LED (e.g., 3 blinks for init failure)
      HAL_GPIO_TogglePin(onboard_led_GPIO_Port, onboard_led_Pin);
      osDelay(200);
      HAL_GPIO_TogglePin(onboard_led_GPIO_Port, onboard_led_Pin);
      osDelay(200);
      HAL_GPIO_TogglePin(onboard_led_GPIO_Port, onboard_led_Pin);
      osDelay(600);
    }
  }
  /* Infinite loop */
  for(;;)
  {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) 
    {
      adc_value = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    float   temp_v = Calculate_Battery_Voltage(adc_value);
    uint8_t temp_p = Calculate_Battery_Percentage(temp_v);
    float   temp_c = MCP9808_ReadTemperature(&hi2c1);

    if (osMutexAcquire(sensorDataMutexHandle, osWaitForever) == osOK)
    {
      sense_ctx.temperature_c = temp_c;
      sense_ctx.battery_voltage = temp_v;
      sense_ctx.battery_percentage = temp_p;
      osMutexRelease(sensorDataMutexHandle);
    }

    osDelay(2000);
  }
  /* USER CODE END StartSenseTask */
}

/* USER CODE BEGIN Header_StartOpticalTask */
/**
* @brief Function implementing the opticalTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartOpticalTask */
void StartOpticalTask(void *argument)
{
  /* USER CODE BEGIN StartOpticalTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_IRDA_Transmit(&hirda1, &optical_context.tx_counter, 1, HAL_MAX_DELAY);
    optical_context.tx_counter++;
    osDelay(1000);
  }
  /* USER CODE END StartOpticalTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM11 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM11)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
