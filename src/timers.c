/*
 * timers.c
 *
 *  Created on: 20 apr 2017
 *      Author: l3uri
 */

#include "timers.h"

void TIM2_PWM_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	Tim2Handle.Instance = TIM2;
	Tim2Handle.Init.Prescaler = 0;
	Tim2Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	Tim2Handle.Init.Period = 8400-1;
	Tim2Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&Tim2Handle);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&Tim2Handle, &sClockSourceConfig);

	HAL_TIM_PWM_Init(&Tim2Handle);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&Tim2Handle,&sMasterConfig);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = Tim2Handle.Init.Period/2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&Tim2Handle,&sConfigOC,TIM_CHANNEL_2);

	if(HAL_TIM_Base_Init(&Tim2Handle) != HAL_OK)
	{
		error_Handler(TIMER2_ERROR);
	}
}

void TIM2ch2_PWM_SetFrqDc(uint32_t freq,uint32_t dc)
{
	Tim2Handle.Init.Period = HCLK/freq;
	HAL_TIM_Base_Init(&Tim2Handle);
	uint32_t pulse = dc * Tim2Handle.Init.Period/4096;
	TIM_OC_InitTypeDef sConfigOC;
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = pulse;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&Tim2Handle,&sConfigOC,TIM_CHANNEL_2);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(htim_base->Instance == TIM2)
	{
		__TIM2_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance == TIM2)
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
	else if (htim_base->Instance == TIM4)
			__TIM4_CLK_DISABLE();
}

void TIM2ch2_PWM_Start()
{
	HAL_TIM_PWM_Start(&Tim2Handle,TIM_CHANNEL_2);
}

void TIM2ch2_PWM_Stop()
{
	HAL_TIM_PWM_Stop(&Tim2Handle,TIM_CHANNEL_2);
}

void TIM4_Init()
{
	__HAL_RCC_TIM4_CLK_ENABLE();

	int uwPrescalerValue = (uint32_t) ((SystemCoreClock/1)/10000)-1;

		Tim4Handle.Instance = TIM4;
		Tim4Handle.Init.Prescaler = uwPrescalerValue;
		Tim4Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
		Tim4Handle.Init.Period = 10-1;
		Tim4Handle.Init.ClockDivision = 0;
		Tim4Handle.Init.RepetitionCounter = 0;

		HAL_NVIC_SetPriority(TIM4_IRQn,1,1);
		HAL_NVIC_EnableIRQ(TIM4_IRQn);

		HAL_TIM_Base_Init(&Tim4Handle);

		if(HAL_TIM_Base_Init(&Tim4Handle) != HAL_OK)
		{
			error_Handler(TIMER4_ERROR);
		}

		if(HAL_TIM_Base_Start_IT(&Tim4Handle) != HAL_OK)
		{
			error_Handler(TIMER4_ERROR);
		}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	if(htim->Instance == TIM2)
	{}
	else if (htim->Instance == TIM4)
	{
			TIM2ch2_PWM_SetFrqDc(1000,HAL_ADC_GetValue(&AdcHandle));
			TIM2ch2_PWM_Start();
	}
}
