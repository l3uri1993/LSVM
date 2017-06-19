/*
 * LPS25HB.c
 *
 *  Created on: 22/feb/2017
 *      Author: flavio
 */
#include "LPS25HB.h"

uint8_t LPS25HB_WhoAmI(void)
{
	const uint8_t	I2C_RXBUFFERSIZE = 1;
	uint8_t I2C_RxBuffer[I2C_RXBUFFERSIZE];

	I2C_RxBuffer[0] = LPS25HB_Who_Am_I;
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)LPS25HB_add<<1 | 1, LPS25HB_Who_Am_I, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	return I2C_RxBuffer[0];
}

void LPS25HB_Config(uint8_t ctrl1, uint8_t ctrl2, uint8_t ctrl3, uint8_t ctrl4)
{
	const uint8_t	I2C_TXBUFFERSIZE = 4;

	uint8_t I2C_TxBuffer[I2C_TXBUFFERSIZE];

	I2C_TxBuffer[0] = ctrl1;
	I2C_TxBuffer[1] = ctrl2;
	I2C_TxBuffer[2] = ctrl3;
	I2C_TxBuffer[3] = ctrl4;

	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)LPS25HB_add<<1 & 0xFE, LPS25HB_CTRL_Reg1, 1, (uint8_t *)&I2C_TxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)LPS25HB_add<<1 & 0xFE, LPS25HB_CTRL_Reg2, 1, (uint8_t *)&I2C_TxBuffer[1], 1, 10000);
	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)LPS25HB_add<<1 & 0xFE, LPS25HB_CTRL_Reg3, 1, (uint8_t *)&I2C_TxBuffer[2], 1, 10000);
	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)LPS25HB_add<<1 & 0xFE, LPS25HB_CTRL_Reg4, 1, (uint8_t *)&I2C_TxBuffer[3], 1, 10000);
}

float LPS25HB_ReadPressure(void)
{
	const uint8_t	I2C_RXBUFFERSIZE = 3;

	uint8_t I2C_RxBuffer[I2C_RXBUFFERSIZE];

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)LPS25HB_add<<1 | 1, LPS25HB_press_XL, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)LPS25HB_add<<1 | 1, LPS25HB_press_L, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)LPS25HB_add<<1 | 1, LPS25HB_press_H, 1, (uint8_t *)&I2C_RxBuffer[2], 1, 10000);
	int32_t data = ((uint32_t)I2C_RxBuffer[2]<<16 | (uint16_t)I2C_RxBuffer[1]<<8 | I2C_RxBuffer[0]);
	return (float)data/4096;
}
