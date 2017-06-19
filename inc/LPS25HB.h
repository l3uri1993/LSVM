/*
 * LPS25HB.h
 *
 *  Created on: 22/feb/2017
 *      Author: flavio
 */

#ifndef LPS25HB_H_
#define LPS25HB_H_

#include "i2c.h"

#define LPS25HB_add			0x5D	//LPS25HB 7bit address (<<1|1 for read, <<1&0xFE for write)
#define LPS25HB_Who_Am_I	0x0F
#define LPS25HB_Who_Am_I_Val	0xBD
#define	LPS25HB_press_XL	0x28
#define	LPS25HB_press_L		0x29
#define	LPS25HB_press_H		0x2A
#define	LPS25HB_ref_press_XL	0x08
#define	LPS25HB_ref_press_L		0x09
#define	LPS25HB_ref_press_H		0x0A
#define LPS25HB_CTRL_Reg1	0x20
#define LPS25HB_CTRL_Reg2	0x21
#define LPS25HB_CTRL_Reg3	0x22
#define LPS25HB_CTRL_Reg4	0x23
#define LPS25HB_Res_Conf	0x10

FunctionalState	LPS25HB_present;
int32_t LPS25HB_RefPressure;

uint8_t LPS25HB_WhoAmI(void);
void	LPS25HB_Config(uint8_t, uint8_t, uint8_t, uint8_t );
float	LPS25HB_ReadPressure(void);

#endif /* LPS25HB_H_ */
