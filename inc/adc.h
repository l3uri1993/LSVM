/*
 * adc.h
 *
 *  Created on: 06 apr 2017
 *      Author: l3uri
 */

#ifndef ADC_H_
#define ADC_H_

#include "stm32f4xx_nucleo.h"
#include "stm32f4xx_hal.h"
#include "main.h"

void ADC1ch6_init(void);
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc);


#endif /* ADC_H_ */
