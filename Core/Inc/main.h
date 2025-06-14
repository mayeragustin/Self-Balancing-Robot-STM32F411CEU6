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
#include "stm32f4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOC
#define KEY_Pin GPIO_PIN_0
#define KEY_GPIO_Port GPIOA
#define LF1_Pin GPIO_PIN_1
#define LF1_GPIO_Port GPIOA
#define LF2_Pin GPIO_PIN_2
#define LF2_GPIO_Port GPIOA
#define LF3_Pin GPIO_PIN_3
#define LF3_GPIO_Port GPIOA
#define LF4_Pin GPIO_PIN_4
#define LF4_GPIO_Port GPIOA
#define WD1_Pin GPIO_PIN_5
#define WD1_GPIO_Port GPIOA
#define WD2_Pin GPIO_PIN_6
#define WD2_GPIO_Port GPIOA
#define WD3_Pin GPIO_PIN_7
#define WD3_GPIO_Port GPIOA
#define WD4_Pin GPIO_PIN_0
#define WD4_GPIO_Port GPIOB
#define BAT_LVL_Pin GPIO_PIN_1
#define BAT_LVL_GPIO_Port GPIOB
#define ESP_EN_Pin GPIO_PIN_2
#define ESP_EN_GPIO_Port GPIOB
#define LedStatus_2_Pin GPIO_PIN_10
#define LedStatus_2_GPIO_Port GPIOB
#define M1_ENC_A_Pin GPIO_PIN_12
#define M1_ENC_A_GPIO_Port GPIOB
#define M1_ENC_A_EXTI_IRQn EXTI15_10_IRQn
#define M2_ENC_A_Pin GPIO_PIN_8
#define M2_ENC_A_GPIO_Port GPIOA
#define M2_ENC_A_EXTI_IRQn EXTI9_5_IRQn
#define M1_IN1_Pin GPIO_PIN_9
#define M1_IN1_GPIO_Port GPIOA
#define M1_IN2_Pin GPIO_PIN_10
#define M1_IN2_GPIO_Port GPIOA
#define M2_IN1_Pin GPIO_PIN_15
#define M2_IN1_GPIO_Port GPIOA
#define M2_IN2_Pin GPIO_PIN_3
#define M2_IN2_GPIO_Port GPIOB
#define M1_PWM_Pin GPIO_PIN_4
#define M1_PWM_GPIO_Port GPIOB
#define M2_PWM_Pin GPIO_PIN_5
#define M2_PWM_GPIO_Port GPIOB
#define ESP_RX_Pin GPIO_PIN_6
#define ESP_RX_GPIO_Port GPIOB
#define ESP_TX_Pin GPIO_PIN_7
#define ESP_TX_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_8
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_9
#define I2C_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
