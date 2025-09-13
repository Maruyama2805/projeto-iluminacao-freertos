/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
extern osThreadId_t threadControleBotId;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BOT_N_Pin GPIO_PIN_13
#define BOT_N_GPIO_Port GPIOC
#define BOT_M_Pin GPIO_PIN_14
#define BOT_M_GPIO_Port GPIOC
#define BOT_L_Pin GPIO_PIN_15
#define BOT_L_GPIO_Port GPIOC
#define LED_1_Pin GPIO_PIN_0
#define LED_1_GPIO_Port GPIOA
#define LED_2_Pin GPIO_PIN_1
#define LED_2_GPIO_Port GPIOA
#define LED_3_Pin GPIO_PIN_2
#define LED_3_GPIO_Port GPIOA
#define BOT_F_Pin GPIO_PIN_3
#define BOT_F_GPIO_Port GPIOA
#define BOT_G_Pin GPIO_PIN_4
#define BOT_G_GPIO_Port GPIOA
#define LED_7_Pin GPIO_PIN_5
#define LED_7_GPIO_Port GPIOA
#define LED_6_Pin GPIO_PIN_6
#define LED_6_GPIO_Port GPIOA
#define BUZZ_Pin GPIO_PIN_0
#define BUZZ_GPIO_Port GPIOB
#define POT_Pin GPIO_PIN_1
#define POT_GPIO_Port GPIOB
#define BOT_B_Pin GPIO_PIN_12
#define BOT_B_GPIO_Port GPIOB
#define BOT_B_EXTI_IRQn EXTI15_10_IRQn
#define BOT_A_Pin GPIO_PIN_13
#define BOT_A_GPIO_Port GPIOB
#define BOT_A_EXTI_IRQn EXTI15_10_IRQn
#define BOT_X_Pin GPIO_PIN_14
#define BOT_X_GPIO_Port GPIOB
#define BOT_X_EXTI_IRQn EXTI15_10_IRQn
#define BOT_Y_Pin GPIO_PIN_15
#define BOT_Y_GPIO_Port GPIOB
#define BOT_Y_EXTI_IRQn EXTI15_10_IRQn
#define LED_5_Pin GPIO_PIN_8
#define LED_5_GPIO_Port GPIOA
#define LED_8_Pin GPIO_PIN_11
#define LED_8_GPIO_Port GPIOA
#define LCD_EN_Pin GPIO_PIN_12
#define LCD_EN_GPIO_Port GPIOA
#define LED_4_Pin GPIO_PIN_15
#define LED_4_GPIO_Port GPIOA
#define BOT_E_Pin GPIO_PIN_3
#define BOT_E_GPIO_Port GPIOB
#define BOT_E_EXTI_IRQn EXTI3_IRQn
#define BOT_D_Pin GPIO_PIN_4
#define BOT_D_GPIO_Port GPIOB
#define BOT_D_EXTI_IRQn EXTI4_IRQn
#define BOT_C_Pin GPIO_PIN_5
#define BOT_C_GPIO_Port GPIOB
#define BOT_C_EXTI_IRQn EXTI9_5_IRQn
#define BOT_H_Pin GPIO_PIN_8
#define BOT_H_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
