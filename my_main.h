#ifndef __MY_MAIN_H
#define __MY_MAIN_H


#include "main.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h> 
#include <limits.h>
#include "lcd.h"
#include "tof.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

void setup(void);
void loop(void);
void uart_serv(char *str);
#endif