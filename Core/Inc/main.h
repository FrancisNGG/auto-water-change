/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WATER_WARNING_Pin GPIO_PIN_1
#define WATER_WARNING_GPIO_Port GPIOB
#define WATER_FULL_Pin GPIO_PIN_11
#define WATER_FULL_GPIO_Port GPIOB
#define WATER_FULL_EXTI_IRQn EXTI15_10_IRQn
#define WATER_LOW_Pin GPIO_PIN_12
#define WATER_LOW_GPIO_Port GPIOB
#define WATER_LOW_EXTI_IRQn EXTI15_10_IRQn
#define WATER_IN_SW_Pin GPIO_PIN_13
#define WATER_IN_SW_GPIO_Port GPIOB
#define WATER_OUT_SW_Pin GPIO_PIN_14
#define WATER_OUT_SW_GPIO_Port GPIOB
#define PLAY_L_Pin GPIO_PIN_15
#define PLAY_L_GPIO_Port GPIOB
#define PLAY_E_Pin GPIO_PIN_8
#define PLAY_E_GPIO_Port GPIOA
#define LCD_TX_Pin GPIO_PIN_9
#define LCD_TX_GPIO_Port GPIOA
#define LCD_RX_Pin GPIO_PIN_10
#define LCD_RX_GPIO_Port GPIOA
#define REC_Pin GPIO_PIN_11
#define REC_GPIO_Port GPIOA
#define IR_IN_Pin GPIO_PIN_6
#define IR_IN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
