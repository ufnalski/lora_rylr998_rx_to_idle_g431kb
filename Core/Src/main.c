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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdlib.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define LORA_PAYLOAD_SIZE 15 // in bytes before representing as string for AT+SEND

typedef union
{
	uint8_t bytes[LORA_PAYLOAD_SIZE];
	struct
	{
		float ahrs_x;  // 4 bytes
		float ahrs_y;  // 4 bytes
		float ahrs_z;  // 4 bytes
		uint8_t enc1;  // 1 byte
		uint8_t enc2;  // 1 byte
		uint8_t button1 :1;  // 1 bit
		uint8_t button2 :1;  // 1 bit
	// 6 bits not used
	};
} lora_payload_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define USE_NANO_SPECS  // CubeIDE -> Project -> Properties -> C/C++ Build -> Settings -> MCU/MPU Settings -> Runtime library -> Reduced C (--specs=nano.specs)
#define LORA_RX_BUFFER_SIZE 128

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */

uint8_t rxData[LORA_RX_BUFFER_SIZE];
uint8_t rxDataBis[LORA_RX_BUFFER_SIZE];

volatile uint8_t lora_data_received_flag = 0;
volatile uint16_t lora_data_received_size = 0;

lora_payload_t payload_to_bytes;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void lora_msg_parser(void);
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
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
	BspCOMInit.BaudRate = 115200;
	BspCOMInit.WordLength = COM_WORDLENGTH_8B;
	BspCOMInit.StopBits = COM_STOPBITS_1;
	BspCOMInit.Parity = COM_PARITY_NONE;
	BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
	if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
	{
		Error_Handler();
	}

	/* USER CODE BEGIN BSP */

	/* -- Sample board code to send message over COM1 port ---- */
	printf("\r\nWelcome to STM32 LoRa world!\r\n");

	/* USER CODE END BSP */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxData, LORA_RX_BUFFER_SIZE);

	while (1)
	{

		if (lora_data_received_flag == 1)
		{
			lora_data_received_flag = 0;
			printf((char*) rxData);
			if (strncmp((char*) rxData, "+RCV=", 5) == 0)
			{
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

				memcpy(rxDataBis, rxData, lora_data_received_size);
				rxDataBis[lora_data_received_size - 2] = '\0';
				memset(rxData, 0x00, LORA_RX_BUFFER_SIZE);
				HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxData,
				LORA_RX_BUFFER_SIZE);
				lora_msg_parser();
				printf("\r\n");
			}
		}
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
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == USART1)
	{
		lora_data_received_flag = 1;
		lora_data_received_size = Size;
	}
}

void lora_msg_parser(void)
{
	char *pt;
	uint8_t cnt = 0;
	pt = strtok((char*) rxDataBis + 5, ",");
	while (pt != NULL)
	{
		int a = atoi(pt);
#ifdef USE_NANO_SPECS
		unsigned int b;
#endif
		switch (cnt)
		{
		case 0:
			printf("Sending node: %d\r\n", a);
			break;
		case 1:
			printf("Payload size: %d\r\n", a);
			break;
		case 2:
			for (uint8_t i = 0; i < LORA_PAYLOAD_SIZE; i++)
			{
#ifdef USE_NANO_SPECS
				if (sscanf(pt + 2 * i, "%02X", &b) == 1)
				{
					payload_to_bytes.bytes[i] = b;
				}
				else
				{
					printf("Something went wrong with sscanf()!");
				}
#else
				if (sscanf(pt + 2 * i, "%2hhX", payload_to_bytes.bytes + i)
						!= 1)
				{
					printf("Something went wrong with sscanf()!");
				}
#endif
			}
			printf("AHRS x: %.1f deg\r\n", payload_to_bytes.ahrs_x);
			printf("AHRS y: %.1f deg\r\n", payload_to_bytes.ahrs_y);
			printf("AHRS z: %.1f deg\r\n", payload_to_bytes.ahrs_z);
			printf("Encoder knob 1: %d\r\n", payload_to_bytes.enc1);
			printf("Encoder knob 2: %d\r\n", payload_to_bytes.enc2);
			printf("Button 1: %d\r\n", payload_to_bytes.button1);
			printf("Button 2: %d\r\n", payload_to_bytes.button2);
			break;
		case 3:
			printf("Received signal strength indicator (RSSI): %d dBm\r\n", a);
			break;
		case 4:
			printf("Signal-to-noise ratio (SNR): %d dB\r\n", a);
			break;
		default:
			__NOP();
		}
		cnt++;
		cnt %= 5;
		pt = strtok(NULL, ",");
	}
}

/* USER CODE END 4 */

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

/**
 * @}
 */

/**
 * @}
 */

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
