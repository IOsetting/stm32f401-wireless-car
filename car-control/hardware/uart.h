#ifndef __UART_H_
#define __UART_H_

#include "stm32f4xx.h"

/**
* UART1: PA9/PA10, PB6/PB7
* UART2: AP2/AP3
* UART6: PA11/PA12
*/
#define PRINT_USARTx   USART1
#define PRINT_GPIOx    GPIOA


void PRINT_USART_Init(void);


#endif
