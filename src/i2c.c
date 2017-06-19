/*
 * i2c.c
 *
 *  Created on: 04 mag 2017
 *      Author: l3uri
 */

#include "i2c.h"

void I2C1_Init()
{
	I2C1Handle.Instance = I2C1;

	I2C1Handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	I2C1Handle.Init.ClockSpeed = 400000;
	I2C1Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2C1Handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
	I2C1Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2C1Handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	if(HAL_I2C_Init(&I2C1Handle) != HAL_OK)
		error_Handler(I2C_ERROR);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_I2C1_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
