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
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


static void LED_Blink(uint32_t Hdelay,uint32_t Ldelay)
{
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_SET);
	HAL_Delay(Hdelay - 1);
	HAL_GPIO_WritePin(E3_GPIO_Port,E3_Pin,GPIO_PIN_RESET);
	HAL_Delay(Ldelay-1);
}


static void RTC_CalendarShow(RTC_DateTypeDef *sdatestructureget,RTC_TimeTypeDef *stimestructureget)
{
  /* ±�?ÐëÍ¬Ê±»ñÈ¡Ê±¼äºÍÈÕÆÚ ²»È»»áµ¼ÖÂÏÂ´ÎRTC²»ÄÜ¶ÁÈ¡ */
  /* Both time and date must be obtained or RTC cannot be read next time */
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&hrtc, stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&hrtc, sdatestructureget, RTC_FORMAT_BIN);
}

uint8_t radio_spi(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint8_t rxData[Size];
	rxData[0] = 0;
	HAL_SPI_TransmitReceive(hspi, pData, (uint8_t*)&rxData, Size, Timeout);
	return rxData[0];
}

void radio_powerup()
{
	// Pull down NREST for 100ms and got to HIGH IMPEDENCE state to reset the radio
	HAL_GPIO_WritePin(GPIOE, NRESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOE, NRESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOE, NRESET_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);
	uint8_t data[2] = {0x8a, 0x00}; // SetPacketType(GFSK)
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	uint8_t status = radio_spi(&hspi1, (uint8_t*)&data, 2, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);
}

void radio_cs(uint8_t cs) {
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, cs ? GPIO_PIN_RESET : GPIO_PIN_SET );
}



void radio_setup() {
	uint8_t text[20];
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);
	// Set packet type to GFSK
	uint8_t data[2] = {0x8a, 00}; // SetPacketType(GFSK)
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	uint8_t status = radio_spi(&hspi1, (uint8_t*)&data, 2, 1000);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	//sprintf((char*)&text, "Hi!");
	LCD_Clear(BLACK);
	sprintf((char *)&text, "%x", status);
	LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 16, text);

	return;

	//LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 3, (char*)&text);
	HAL_Delay(1000);

	HAL_Delay(100);
	// Set 2.4 Ghz centre frequency
	uint32_t rfFrequency = 12098954;
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&rfFrequency, 3, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	// Set buffer base address
	uint8_t data2[3] = {0x8f, 00, 00};
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data2, 3, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	// Set modulation parameters
	uint8_t data3[4] = {0x8B, 0x04, 0x00, 00};
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data3, 4, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	// Set packet type to GFSK
	uint8_t data4[2] = {0x8A, 0x00};
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data4, 2, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	// Set packet parameters
	uint8_t data5[8] = {0x8C, 0x5, 0x00, 0x00, 0x00, 0x06, 0x00, 0x08};
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data5, 8, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

}

void radio_tx(uint8_t* buffer) {
	HAL_Delay(100);
	// Set buffer base address
	uint8_t data[3] = {0x8f, 00, 00};
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data, 3, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	// SetTxParams(power = -12 dBm, ramp_time = 2 us)
	uint8_t data2[3] = {0x8E, 0x5, 0x00};
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data2, 3, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	// Copy data to buffer with OP code prefix
	uint8_t data3[8];
	data3[0] = 0x1A; // WriteBuffer OP code prefix
	data3[1] = 0x00;
	data3[2] = buffer[0];
	data3[3] = buffer[1];
	data3[4] = buffer[2];
	data3[5] = buffer[3];
	data3[6] = buffer[4];
	data3[7] = buffer[5];
	//for (int i = 2; i < 8; ++i) {
	//	data[i] = buffer[i-2];
	//}

	HAL_Delay(100);
	// WriteBuffer
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data3, 8, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);

	HAL_Delay(100);
	// SetTx(periodBase = 0 ms, timeout_duration = 0)
	uint8_t data4[3] = {0x83, 0x00, 0x00};
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&data4, 3, 100);
	HAL_GPIO_WritePin(GPIOE, NSS_Pin, GPIO_PIN_SET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  uint8_t data[6] = {1, 2, 3, 5, 8, 13};
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
  MX_RTC_Init();
  MX_SPI4_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(E3_GPIO_Port,GPIO_PIN_10,GPIO_PIN_RESET);
  //LCD_Test();
  LCD_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  	uint8_t text[20];
  	RTC_DateTypeDef sdatestructureget;
  	RTC_TimeTypeDef stimestructureget;

  	// Initialize radio
  	HAL_GPIO_WritePin(E3_GPIO_Port,GPIO_PIN_10,GPIO_PIN_RESET);
  	radio_powerup();
  	radio_setup();

    //sprintf((char *)&text, "To seek is to find me...");
    //LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 16, text);
  	while (1)
  	{

  		radio_setup();
  	}
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    	HAL_GPIO_WritePin(E3_GPIO_Port,GPIO_PIN_10,GPIO_PIN_RESET);

  		RTC_CalendarShow(&sdatestructureget,&stimestructureget);

  		if (stimestructureget.Seconds % 2 == 1)
  			sprintf((char *)&text,"Time: %02d:%02d", stimestructureget.Hours, stimestructureget.Minutes);
  		else
  			sprintf((char *)&text,"Time: %02d %02d", stimestructureget.Hours, stimestructureget.Minutes);
  		//LCD_ShowString(x, y, width, height, size, p);

  		sprintf((char *)&text,"Tick: %d ms",HAL_GetTick());
  		LCD_ShowString(4, 74, 160, 16, 16,text);

  		LED_Blink(3,500);

  		HAL_Delay(100);
  		radio_tx((uint8_t*)&data);
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
