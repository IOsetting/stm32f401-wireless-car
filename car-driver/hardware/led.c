#include <stdio.h>
#include "led.h"

void LED_Init(void)
{
  if (LED_GPIOx == GPIOA) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  } else if (LED_GPIOx == GPIOB) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  } else if (LED_GPIOx == GPIOC) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  }
  GPIO_InitTypeDef  gpioInitTypeDef;
	gpioInitTypeDef.GPIO_Pin   = LED_GPIO_PIN;
	gpioInitTypeDef.GPIO_Mode  = GPIO_Mode_OUT;
	gpioInitTypeDef.GPIO_OType = GPIO_OType_PP;
	gpioInitTypeDef.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(LED_GPIOx, &gpioInitTypeDef);
  GPIO_SetBits(LED_GPIOx, LED_GPIO_PIN);
  printf("## LED Initialized ##\r\n");
}

void LED_Off(void)
{
  GPIO_SetBits(LED_GPIOx, LED_GPIO_PIN);
}

void LED_On(void)
{
  GPIO_ResetBits(LED_GPIOx, LED_GPIO_PIN);
}
