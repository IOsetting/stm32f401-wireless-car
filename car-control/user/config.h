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

#endif
