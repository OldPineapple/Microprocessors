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
#include "arm_math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ITM_Port32(n) (*((volatile unsigned long *) (0xE0000000+4*n)))
uint8_t triangle;
uint8_t saw;
uint8_t sine;
// sample rate: 44.1kHz, system clock freq: 120MHz, counter period
#define first_sample_number 44	// 44.1kHz / 1kHz = 44, C6
#define second_sample_number 33	// 44.1kHz / 1.318kHz = 33, E6
#define third_sample_number 22 // 44.1kHz / 22 = 2.005kHz
#define buffer_length 132	// Buffer length
uint8_t first_array[buffer_length];	// This is the array used to store all C6 values in a period
uint8_t second_array[buffer_length]; // This is the array used to store all E6 values in a period
uint8_t third_array[buffer_length]; // This is the array used to store all values of the third frequency in a period
uint8_t buffer_array[buffer_length]; // This is the array used to store all buffer values in a period
int number = 0;	// This is the global variable to examine the current position
int tone = 0;	// This is the global variable to examine the current tone played
int currentTone = 3;	// This is the global variable to record the current tone
int previousButtonState = GPIO_PIN_RESET;	//This is the global variable to store the previous button state(not pressed)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef * htim);
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef * hdac);
void HAL_DAC_ConvCpltCallbackCh1 (DAC_HandleTypeDef * hdac);
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
  MX_DMA_Init();
  MX_DAC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  // Part 2
  int loop = 0;
  float32_t counter = 0;
  float32_t radian = 0;
  float32_t temp = 0;
  for (loop = 0; loop < 3; loop++) {
	  for (counter = 0; counter < first_sample_number; counter++) {
	  	  radian = counter * 8.2 / 180 * PI;	// 360 / 44 = 8.2
	  	  temp = arm_sin_f32(radian); // -1 to 1
	  	  sine = (uint8_t)((temp + 1) * 127 * (2.0 / 3)); // change -1 to 1 to 0 to 2 then scale to 0 to 254, Scale your DAC values so they vary over about 2/3 of the possible dynamic range
	  	  first_array[44 * loop + (int)counter] = sine;
	    }
  }

  for (loop = 0; loop < 4; loop++) {
	  for (counter = 0; counter < second_sample_number; counter++) {
	  	  radian = counter * 10.9 / 180 * PI;	// 360 / 33 = 10.9
	  	  temp = arm_sin_f32(radian); // -1 to 1
	  	  sine = (uint8_t)((temp + 1) * 127 * (2.0 / 3)); // change -1 to 1 to 0 to 2 then scale to 0 to 254, Scale your DAC values so they vary over about 2/3 of the possible dynamic range
	  	  second_array[33 * loop + (int)counter] = sine;
	    }
  }

  for (loop = 0; loop < 6; loop++) {
	  for (counter = 0; counter < third_sample_number; counter++) {
	  	  radian = counter * 16.4 / 180 * PI;	// 360 / 22 = 16.4
	      temp = arm_sin_f32(radian); // -1 to 1
	      sine = (uint8_t)((temp + 1) * 127 * (2.0 / 3)); // change -1 to 1 to 0 to 2 then scale to 0 to 254, Scale your DAC values so they vary over about 2/3 of the possible dynamic range
	      third_array[22 * loop + (int)counter] = sine;
	    }
  }

  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1); // ARD.D7
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_2); // ARD.D13-SPI1_SCK/LED1
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Part 1
	  // generate signal with a period of ~15ms
	  float32_t radian = 0;
	  float32_t temp = 0;
	  for (uint8_t counter = 0; counter < 16; counter++) { // Max should be 15 (15ms)
		  // triangle calculation
		  if (counter < 8) {
			  triangle = counter * 32; // 255/8 = 31.875
		  }
		  else if (counter == 8) {
			  triangle = 255; // max value for 8 bit
		  }
		  else {
			  triangle = (16 - counter) * 32;
		  }
		  // saw calculation
		  saw = counter * 17;	// 255/15ms = 17
		  radian = (float32_t)counter * 24 / 180 * PI; // 360/15 = 24
		  temp = arm_sin_f32(radian); // -1 to 1
		  sine = (uint8_t)((temp + 1) * 127); //change -1 to 1 to 0 to 2 then scale to 0 to 254
		  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1,  DAC_ALIGN_8B_R, triangle);
		  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2,  DAC_ALIGN_8B_R, saw);
		  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1,  DAC_ALIGN_8B_R, sine);
		  HAL_Delay(0.4);
	  }

	  // Part 2 Step 1
//	  int currentButtonState = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);
//	  if (currentButtonState != previousButtonState) {
//		  if (currentButtonState == GPIO_PIN_RESET) {
//			  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
//		  }
//		  previousButtonState = currentButtonState;
//	  }
//
//	  // Part 2 Step 2
//
//	  if (HAL_DAC_GetState(&hdac1) == HAL_DAC_STATE_READY) {
//		  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1,  DAC_ALIGN_8B_R, first_array[number]);
//	  }
//
//
//	  // Part 2 Step 4
//	  if (currentTone != tone) {
//		  HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);	// Stop the previous tone
//		  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, buffer_array, 132, DAC_ALIGN_8B_R);	// Play the next tone
//		  currentTone = tone;
//	  }

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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
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
}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */
  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2721;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {	// detect when the button is pressed
//	if (GPIO_Pin == USER_BUTTON_Pin) {
//		HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
//		tone++;
//		if (tone == 3) {
//			tone = 0;
//		}
//	}
//}
//
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {	// What happens when timer reaches its counter number
//	if (htim == &htim2) {
//		number++;
//		if (number == buffer_length) {
//			number = 0;
//		}
//	}
//}
//
//void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef * hdac) {	// load second half of the array
//	  if (tone == 0) {
//		  for (int i = 66; i < buffer_length; i++) {
//			  buffer_array[i] = first_array[i];
//		  }
//	  }
//	  else if (tone == 1) {
//		  for (int i = 66; i < buffer_length; i++) {
//			  buffer_array[i] = second_array[i];
//		  }
//	  }
//	  else if (tone == 2) {
//		  for (int i = 66; i < buffer_length; i++) {
//			  buffer_array[i] = third_array[i];
//		  }
//	  }
//}
//
//void HAL_DAC_ConvCpltCallbackCh1 (DAC_HandleTypeDef * hdac) {	// load first half of the array
//	  if (tone == 0) {
//		  for (int i = 0; i < buffer_length / 2; i++) {
//			  buffer_array[i] = first_array[i];
//		  }
//	  }
//	  else if (tone == 1) {
//		  for (int i = 0; i < buffer_length / 2; i++) {
//			  buffer_array[i] = second_array[i];
//		  }
//	  }
//	  else if (tone == 2) {
//		  for (int i = 0; i < buffer_length / 2; i++) {
//			  buffer_array[i] = third_array[i];
//		  }
//	  }
//}
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
