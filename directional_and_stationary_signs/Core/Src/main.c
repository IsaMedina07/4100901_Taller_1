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
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
UART_HandleTypeDef huart2;
uint32_t last_press_time[3] = {0}; 	// Array que almacena el tiempo de A1, A2 y A3
bool leftLightBlinking = false;		// Variable que indica si oprimieron dos veces o no la direccional izquierda
bool rightLightBlinking = false;	// Variable que indica si oprimieron dos veces o no la direccional derecha
uint32_t last_double_press = 0;		// Último registro de doble pulsación

// Contadores para las veces que las direccionales van a parpadear
uint32_t cont_left = 0;
uint32_t cont_right = 0;
uint32_t into_stationary = 0;

uint8_t index_pines = 0; //índice botónn presionado

bool status_stationary = false; //variable booleana que indica el estado de las luces estacionarias


/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  uint32_t current_time = HAL_GetTick();	// Tiempo transcurrido de la última pulsación
  uint8_t index_btn;						// índice del botón preionado

  // Para identificar el botón presionado:
  if(GPIO_Pin == A1_Pin){
	  index_btn = 0;
	  index_pines = 0;
  }else if(GPIO_Pin == A2_Pin){
	  index_btn = 1;
	  index_pines = 1;
  }else if(GPIO_Pin == A3_Pin){
	  index_btn = 2;
	  index_pines = 2;
  }else{
	  HAL_UART_Transmit(&huart2, "NULL\r\n", 6,10);
	  return;
  }

  // se comprueba el tiempo de la última pulsación (Que no se pase de 200ms para evitar el "rebote"):
  if(current_time - last_press_time[index_btn] > 200){
	  last_press_time[index_btn] = current_time; // Actualiza el tiempo


	  // Se comprueba si se ha oprimido alguna direccional (izquierda o derecha) y que las estacionarias no estén encendidas:
	  if((index_btn == 0 || index_btn == 1) && (!status_stationary)){
		  uint32_t current_double_time = HAL_GetTick(); // Tiempo de las dobles pulsaciones

		  // Revisión si no se realizó dos veces la última pulsación en un tiempo menor o igual a los 300ms
		  if (current_double_time - last_double_press <= 300) {
			  // Si se realizó la doble pulsación :
			  if (index_btn == 0) {
			      leftLightBlinking = true;		// Encender indefinidamente la direccional izquierda
			      rightLightBlinking = false;	// Apagar la derecha si estaba encendida
			      HAL_UART_Transmit(&huart2, "Intro double Izquierda\r\n", 24,10);
			  } else {
				  rightLightBlinking = true;	// Encender indefinidamente la direccional derecha
				  leftLightBlinking = false;	// Apagar la izquierda si estaba encendida
				  HAL_UART_Transmit(&huart2, "Intro double Derecha\r\n", 22,10);
			  }
		  }else {
		      //Si no se realizó la doble pulsación :
			  last_double_press = current_double_time;

			  // Vemos si alguna variable de doble pulsación está activada para desactivarla:
			  if(rightLightBlinking || leftLightBlinking){

				  // Lógica para las direccionales de forma indefinida:

				  // Si la direccional derecha está encendida y se oprime el botón izquierdo:
				  if(index_btn == 0 && rightLightBlinking){
					  // Se apaga completamente la direccional derecha
					  rightLightBlinking = false;
					  cont_right = 0;
					  HAL_UART_Transmit(&huart2, "Apagado de la direccional indefinida Derecha\r\n", 46,10);
				  }
				  // Si la direccional izquierda está encendida y se oprime el botón derecho:
				  else if(index_btn == 1 && leftLightBlinking){
					  // Se apaga completamente la direccional izquierda
					  leftLightBlinking = false;
					  cont_left = 0;
					  HAL_UART_Transmit(&huart2, "Apagado de la direccional indefinida Izquierda\r\n", 48,10);
				  }
				  /*
				  de lo contrario no se realiza ninguna acción y se mantiene la direccional encendida.
				  */
			  }else{
				  /* Si no hay doble pulsación y no se tienen encendidas las direcionales de forma indefinida
				   * se realizan las acciones de las direccionales de forma "normal", se encienden tres veces.
				   */
				  if(index_btn == 0){
					  cont_left = 6;
					  cont_right = 0; // Para parar el derecho cuando se presione el izquierdo
					  HAL_UART_Transmit(&huart2, "Direccional izquierda\r\n", 23,10);
				  }else if(index_btn == 1){
					   cont_right = 6;
					   cont_left = 0; // Para parar el izquierdo cuando se presione el derecho
					  HAL_UART_Transmit(&huart2, "Direccional derecha\r\n", 21,10);
				  }
			  }
		    }
	  }else if (index_btn == 2) {
	            // Alternar estado de las luces estacionarias
		   if (!status_stationary) {
			// Encender las luces estacionarias
			   status_stationary = true;
			 }else {
				// Apagar las luces estacionarias
				status_stationary = false;
				index_pines = 0;
			 	}
		}
  	  }
 }

// Configuración del hearbeat:
void heartbeat(void){
	static uint32_t tick = 0;
	if(tick < HAL_GetTick()){
		tick = HAL_GetTick() + 1000;
		HAL_GPIO_TogglePin(D4_GPIO_Port, D4_Pin);
	}
}

void turn_signal_led_left(void){
	static uint32_t left_tick = 0;
	if(left_tick < HAL_GetTick()){
		if(cont_left > 0 || leftLightBlinking){
			left_tick =  HAL_GetTick() + 500;
			HAL_GPIO_TogglePin(D1_GPIO_Port, D1_Pin);
			/*// Si la direccional se enciende por leftLightBlinking = true,
			 * el contador no afectará hasta que se le asigne un número mayor a cero
			 * y leftLightBlinking = false (Lo que pasaría si se oprime solo una vez la direccional).
			 * Lo mismo pasa con la función de la direccional derecha.*/
			cont_left--;
		}else{
			HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, 1);
		}
	}
}

void turn_signal_led_right(void){
	static uint32_t right_tick = 0;
	if(right_tick < HAL_GetTick()){
		if(cont_right > 0 || rightLightBlinking){
			right_tick = HAL_GetTick() + 500;
			HAL_GPIO_TogglePin(D2_GPIO_Port, D2_Pin);
			cont_right--;
		}else{
		HAL_GPIO_WritePin(D2_GPIO_Port, D2_Pin, 1);
		}
	}
}

//Se configura el comportamiento de las luces estacionarias.
void stationary(void){
	static uint32_t es_tick = 0;
	if(es_tick < HAL_GetTick()){
		if(status_stationary){
			es_tick =  HAL_GetTick() + 500;
			HAL_GPIO_TogglePin(D1_GPIO_Port, D1_Pin);
			HAL_GPIO_TogglePin(D2_GPIO_Port, D2_Pin);
		}else{
			HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, 1);
			HAL_GPIO_WritePin(D2_GPIO_Port, D2_Pin, 1);
		}
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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  heartbeat();

	  	if(index_pines == 2){
	  		stationary();
	  		HAL_UART_Transmit(&huart2, "Into\r\n", 6,10);
	  	}else{
	      turn_signal_led_left();
	      turn_signal_led_right();
	  	}
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, D1_Pin|D2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : A1_Pin A2_Pin */
  GPIO_InitStruct.Pin = A1_Pin|A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : D1_Pin D2_Pin */
  GPIO_InitStruct.Pin = D1_Pin|D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : A3_Pin */
  GPIO_InitStruct.Pin = A3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(A3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : D4_Pin */
  GPIO_InitStruct.Pin = D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(D4_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

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
