#include <stdio.h>
#include "uart.h"

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  while(USART_GetFlagStatus(PRINT_USARTx, USART_FLAG_TXE) == RESET){}
  USART_SendData(PRINT_USARTx, ch);
  return(ch);
}

int fgetc(FILE *f) {
  char ch;
  while(USART_GetFlagStatus(PRINT_USARTx, USART_FLAG_RXNE) == RESET){}
  ch = USART_ReceiveData(PRINT_USARTx);
  return((int)ch);
}

int ferror(FILE *f) {
  return EOF;
}

void _ttywrch(int ch) {
  while(USART_GetFlagStatus(PRINT_USARTx, USART_FLAG_TXE) == RESET){}
  USART_SendData(PRINT_USARTx, ch);
}

void _sys_exit(int return_code) {
  while (1); /* endless loop */
}

/**
 * UART PRINT Init
 */
void PRINT_USART_Init(void)
{
  if (PRINT_USARTx == USART1) {
    if (PRINT_GPIOx == GPIOA) {
      // UART1: PA9/PA10
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
      GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
      GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

      GPIO_InitTypeDef GPIO_InitStructure;
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
      GPIO_Init(GPIOA, &GPIO_InitStructure);

    } else if (PRINT_GPIOx == GPIOB) {
      // UART1: PB6/PB7
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
      GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
      GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

      GPIO_InitTypeDef GPIO_InitStructure;
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
      GPIO_Init(GPIOB, &GPIO_InitStructure);

    }
    USART_DeInit(USART1);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_ClearFlag(USART1, USART_FLAG_CTS);
    USART_Cmd(USART1, ENABLE);
    printf("## UART1 Initialized ##\r\n");

  } else if (PRINT_USARTx == USART2) {
    // UART2: AP2, AP3
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_DeInit(USART2);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    USART_ClearFlag(USART2, USART_FLAG_CTS);
    USART_Cmd(USART2, ENABLE);
    printf("## UART2 Initialized ##\r\n");

  } else if (PRINT_USARTx == USART6) {
    // UART6: PA11/PA12
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_USART6);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_USART6);

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_DeInit(USART6);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART6, &USART_InitStructure);
    USART_ClearFlag(USART6, USART_FLAG_CTS);
    USART_Cmd(USART6, ENABLE);
    printf("## UART6 Initialized ##\r\n");

  }

}


