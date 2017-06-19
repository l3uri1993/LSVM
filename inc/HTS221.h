/*
 * HTS221.h
 *
 *  Created on: 22/feb/2017
 *      Author: flavio
 */
#ifndef HTS221_H_
#define HTS221_H_

#include "i2c.h"

#define HTS221_add			0x5F	//HTS221 7bit address (<<1|1 for read, <<1&0xFE for write)
#define HTS221_Who_Am_I		0x0F
#define HTS221_Who_Am_I_Val	0xBC
#define HTS221_AV_Conf		0x10
#define HTS221_CTRL_Reg1	0x20
#define HTS221_CTRL_Reg2	0x21
#define HTS221_CTRL_Reg3	0x22
#define HTS221_Stat_Reg		0x27
#define HTS221_HumidityL	0x28
#define HTS221_HumidityH	0x29
#define HTS221_TempL		0x2A
#define HTS221_TempH		0x2B

FunctionalState	HTS221_present;
int16_t HTS221_H0_T0_Out, HTS221_H1_T0_Out, HTS221_T0_Out, HTS221_T1_Out, HTS221_T0_degC_x8, HTS221_T1_degC_x8;
uint8_t HTS221_H0_rHx2, HTS221_H1_rHx2;
float mT,mH;

void	HTS221_Config(uint8_t, uint8_t, uint8_t, uint8_t );
uint8_t	HTS221_WhoAmI(void);
float	HTS221_ReadHumidity(void);
float	HTS221_ReadTemperature(void);
void	HTS221_ReadCalib(void);

#endif /* HTS221_H_ */
