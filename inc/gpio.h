#ifndef GPIO_H
#define GPIO_H

#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_hal.h"
#include "main.h"

#define LED_ESTERNO GPIO_PIN_5
#define BUTTON_ESTERNO GPIO_PIN_0

void GPIO_Init();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif
