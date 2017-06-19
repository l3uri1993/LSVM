/*
 * HTS221.c
 *
 *  Created on: 22/feb/2017
 *      Author: flavio
 */
#include "HTS221.h"

float HTS221_ReadHumidity(void)
{
	const uint8_t	I2C_RXBUFFERSIZE = 2;

	uint8_t I2C_RxBuffer[I2C_RXBUFFERSIZE];
	float Humidity;

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, HTS221_HumidityL, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, HTS221_HumidityH, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);
	int16_t data = (((uint16_t)I2C_RxBuffer[1]<<8 | I2C_RxBuffer[0]));
	Humidity = (float)data*mH+(float)HTS221_H0_rHx2/2;
	return Humidity;
}

float HTS221_ReadTemperature(void)
{
	const uint8_t	I2C_RXBUFFERSIZE = 2;

	uint8_t I2C_RxBuffer[I2C_RXBUFFERSIZE];
	float Temperature;

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, HTS221_TempL, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, HTS221_TempH, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);
	int16_t data = (((uint16_t)I2C_RxBuffer[1]<<8 | I2C_RxBuffer[0]));
	Temperature = (float)data*mT+(float)HTS221_T0_degC_x8/8;
	return Temperature;
}

void HTS221_Config(uint8_t AVconf, uint8_t ctrl1, uint8_t ctrl2, uint8_t ctrl3)
{
	const uint8_t	I2C_TXBUFFERSIZE = 4;

	uint8_t I2C_TxBuffer[I2C_TXBUFFERSIZE];

	I2C_TxBuffer[0] = AVconf;
	I2C_TxBuffer[1] = ctrl1;
	I2C_TxBuffer[2] = ctrl2;
	I2C_TxBuffer[3] = ctrl3;

	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)HTS221_add<<1 & 0xFE, HTS221_AV_Conf, 1, (uint8_t *)&I2C_TxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)HTS221_add<<1 & 0xFE, HTS221_CTRL_Reg1, 1, (uint8_t *)&I2C_TxBuffer[1], 1, 10000);
	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)HTS221_add<<1 & 0xFE, HTS221_CTRL_Reg2, 1, (uint8_t *)&I2C_TxBuffer[2], 1, 10000);
	HAL_I2C_Mem_Write(&I2C1Handle, (uint16_t)HTS221_add<<1 & 0xFE, HTS221_CTRL_Reg3, 1, (uint8_t *)&I2C_TxBuffer[3], 1, 10000);

}

void HTS221_ReadCalib(void)
{
	const uint8_t	I2C_RXBUFFERSIZE = 3;

	uint8_t I2C_RxBuffer[I2C_RXBUFFERSIZE];

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x30, 1, (uint8_t *)&HTS221_H0_rHx2, 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x31, 1, (uint8_t *)&HTS221_H1_rHx2, 1, 10000);

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x32, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x33, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);;
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x35, 1, (uint8_t *)&I2C_RxBuffer[2], 1, 10000);
	HTS221_T0_degC_x8 = ((uint16_t)I2C_RxBuffer[2]<<8 & 0x300) | I2C_RxBuffer[0];
	HTS221_T1_degC_x8 = ((uint16_t)I2C_RxBuffer[2]<<6 & 0x300) | I2C_RxBuffer[1];

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x36, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x37, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);
	HTS221_H0_T0_Out = (uint16_t)I2C_RxBuffer[1]<<8 | I2C_RxBuffer[0];

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x3A, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x3B, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);
	HTS221_H1_T0_Out = (uint16_t)I2C_RxBuffer[1]<<8 | I2C_RxBuffer[0];

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x3C, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x3D, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);
	HTS221_T0_Out = (uint16_t)I2C_RxBuffer[1]<<8 | I2C_RxBuffer[0];

	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x3E, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, 0x3F, 1, (uint8_t *)&I2C_RxBuffer[1], 1, 10000);
	HTS221_T1_Out = (uint16_t)I2C_RxBuffer[1]<<8 | I2C_RxBuffer[0];

	mT=(((float)HTS221_T1_degC_x8/8)-((float)HTS221_T0_degC_x8/8))/(HTS221_T1_Out-HTS221_T0_Out);
	mH=(((float)HTS221_H1_rHx2/2)-((float)HTS221_H0_rHx2/2))/(HTS221_H1_T0_Out-HTS221_H0_T0_Out);

}

uint8_t HTS221_WhoAmI(void)
{
	const uint8_t	I2C_RXBUFFERSIZE = 1;
	uint8_t I2C_RxBuffer[I2C_RXBUFFERSIZE];

	I2C_RxBuffer[0] = HTS221_Who_Am_I;
	HAL_I2C_Master_Transmit(&I2C1Handle, (uint16_t)HTS221_add<<1  & 0xFE, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	HAL_I2C_Master_Receive(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	//HAL_I2C_Mem_Read(&I2C1Handle, (uint16_t)HTS221_add<<1 | 1, HTS221_Who_Am_I, 1, (uint8_t *)&I2C_RxBuffer[0], 1, 10000);
	return I2C_RxBuffer[0];
}

