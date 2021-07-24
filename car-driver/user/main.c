#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "led.h"
#include "timer.h"
#include "uart.h"
#include "nrf24l01.h"

u8 RX_ADDRESS[NRF24L01_ADDR_WIDTH] = {0x32,0x4E,0x6F,0x64,0x65};
u8 TX_ADDRESS[NRF24L01_ADDR_WIDTH] = {0x11,0x22,0x33,0x44,0x55};

extern u8 RX_BUF[];
extern u8 TX_BUF[];


void EXTI15_10_IRQHandler(void) {
  LED_On();
  /* Make sure that interrupt flag is set */
  if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
    NRF24L01_IRQ_Handler(RX_BUF);
    /* Clear interrupt flag */
    EXTI_ClearITPendingBit(EXTI_Line13);
  }
  LED_Off();
}


int main(void)
{
  Systick_Init();
  USART1_Init();
  NRF24L01_Init();
  while(NRF24L01_Check() != 0) {
    printf("nRF24L01 check failed\r\n");
    Systick_Delay_ms(2000);
  }
  printf("nRF24L01 check succeeded\r\n");
  NRF24L01_RX_Mode(RX_ADDRESS, TX_ADDRESS);
  printf("nRF24L01 in RECEIVE mode\r\n");
  NRF24L01_DumpConfig();

  LED_Init();
  while(1) {}
}

