#ifndef MAIN_H
#define MAIN_H

#include "cmsis_os.h"

#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"
#include "usart.h"
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
#define CLOCK_ERROR 2

#define HCLK 84000000
#define WIFI 1
#define TERMINAL 2

uint8_t aRxBuffer2[1];
uint8_t aRxBuffer1[1];

UART_HandleTypeDef UARTHandle1;
UART_HandleTypeDef UARTHandle2;
UART_HandleTypeDef printfPort;

void error_Handler(int error);

void SystemClock_Config(void);

void mainThread(void const *argument);

int split_file_load(char *f);

int libsvm_load_data(char *filename);

int binary_load_data(char *filename);

void load_data_file(char *filename);

int count_svs();

int libsvm_save_model(const char *model_file_name);

float kernel(int i, int j, void *kparam);

void finish(lasvm_t *sv);

void make_old(int val);

int selectstrategy(lasvm_t *sv);

void train_online(char *model_file_name);

void mainThread(void const *argument);

#endif
