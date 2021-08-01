#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "config.h"
#include "led.h"
#include "timer.h"
#include "uart.h"
#include "nrf24l01.h"
#include "adc.h"

u8 RX_ADDRESS[NRF24L01_ADDR_WIDTH] = {0x11,0x22,0x33,0x44,0x55};
u8 TX_ADDRESS[NRF24L01_ADDR_WIDTH] = {0x32,0x4E,0x6F,0x64,0x65};

extern u8 RX_BUF[];
extern u8 TX_BUF[];

extern u16 ADCConvertedValue[];

int main(void)
{
  Systick_Init();

  PRINT_USART_Init();

  NRF24L01_Init();
  NRF24L01_DumpConfig();
  while(NRF24L01_Check() != 0) {
    printf("nRF24L01 check failed\r\n");
    Systick_Delay_ms(2000);
  }
  printf("nRF24L01 check succeeded\r\n");

  printf("nRF24L01 in SEND mode\r\n");
  NRF24L01_TX_Mode(RX_ADDRESS, TX_ADDRESS);

  LED_Init();
  
  ADC_Initialize();

  while(1) {
    u8 tor = 5;
    //NRF24L01_DumpConfig();
    // Calculate the average value of X and Y
    u16 axis_x = 0, axis_y = 0;
    for (u8 i = 0; i < ARRAYSIZE / 2; i++) {
      axis_x += *(ADCConvertedValue + i*2) / 16;
      axis_y += *(ADCConvertedValue + i*2 + 1) / 16;
    }
    axis_x = (axis_x * 2) / ARRAYSIZE;
    axis_y = (axis_y * 2) / ARRAYSIZE;
    printf("%X, %X\r\n", axis_x, axis_y);
    if (axis_x >= 0x7f - tor && axis_x <= 0x7f + tor
      && axis_y >= 0x7f - tor && axis_y <= 0x7f + tor) {

      printf("skipped\r\n");
      Systick_Delay_ms(200);
      continue;
    }

    u8 tmp[] = {0x02, (u8)axis_x, (u8)axis_y};
    u8 status = NRF24L01_TxPacket(tmp, 32);
    if(status & NRF24L01_FLAG_TX_DSENT) {
      LED_On();
      Systick_Delay_ms(200);
      LED_Off();
      Systick_Delay_ms(200);
    } else {
      Systick_Delay_ms(400);
    }
  }
}

