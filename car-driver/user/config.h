#ifndef _CONFIG_H
#define _CONFIG_H

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

#define LED_GPIOx GPIOC
#define LED_GPIO_PIN GPIO_Pin_13

#define NRF24L01_SPIx     SPI1
#define NRF24L01_GPIOx    GPIOB
#define NRF24L01_GPIO_IRQ GPIO_Pin_13
#define NRF24L01_GPIO_CE  GPIO_Pin_14
#define NRF24L01_GPIO_CSN GPIO_Pin_15

#define NRF24L01_INT_MODE 1

static u8 RX_ADDRESS[] = {0x32,0x4E,0x6F,0x64,0x65};
static u8 TX_ADDRESS[] = {0x11,0x22,0x33,0x44,0x55};

#endif
