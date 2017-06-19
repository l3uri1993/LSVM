/*
 * usart.h
 *
 *  Created on: 06 apr 2017
 *      Author: l3uri
 */

#ifndef USART_H_
#define USART_H_

#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_hal.h"
#include "main.h"

void select_USART(char select);

void USART2_Init(void);
void USART1_Init(void);

#endif /* USART_H_ */
