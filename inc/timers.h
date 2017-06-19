/*
 * timers.h
 *
 *  Created on: 20 apr 2017
 *      Author: l3uri
 */

#ifndef TIMERS_H_
#define TIMERS_H_

#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_hal.h"
#include "main.h"

void TIM2_PWM_Init();
void TIM2ch2_PWM_SetFrqDc(uint32_t freq,uint32_t dc);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base);
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base);
void TIM2ch2_PWM_Start();
void TIM2ch2_PWM_Stop();
void TIM4_Init();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);


#endif /* TIMERS_H_ */
