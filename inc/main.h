#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include "gpio.h"
#include "usart.h"
#include "adc.h"
#include "timers.h"
#include "i2c.h"
#include "HTS221.h"
#include "LPS25HB.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "dyn_arrays.h"
#include "vector.h"
#include "lasvm.h"

#define LINEAR  0
#define POLY    1
#define RBF     2
#define SIGMOID 3

#define ONLINE 0
#define ONLINE_WITH_FINISHING 1

#define RANDOM 0
#define GRADIENT 1
#define MARGIN 2

#define ITERATIONS 0
#define SVS 1
#define TIME 2

#define UART_ERROR 1
#define ADC_ERROR 2
#define CLOCK_ERROR 3
#define TIMER2_ERROR 4
#define TIMER4_ERROR 5
#define I2C_ERROR 6

#define HCLK 84000000
#define WIFI 1
#define TERMINAL 2

uint16_t debounce;
uint8_t BtnInt;
uint8_t aRxBuffer2[1];
uint8_t aRxBuffer1[1];

GPIO_InitTypeDef  GPIO_InitStruct;
UART_HandleTypeDef UARTHandle1;
UART_HandleTypeDef UARTHandle2;
UART_HandleTypeDef printfPort;
ADC_HandleTypeDef AdcHandle;
TIM_HandleTypeDef Tim2Handle;
TIM_HandleTypeDef Tim4Handle;
I2C_HandleTypeDef I2C1Handle;

void error_Handler(int error);

void SystemClock_Config(void);

#endif
