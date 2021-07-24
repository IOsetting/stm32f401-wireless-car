#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "led.h"
#include "timer.h"
#include "uart.h"
#include "nrf24l01.h"

extern u8 RX_BUF[];
extern u8 TX_BUF[];

u16 TimerPeriod = 0;
u16 Channel1Pulse = 0, Channel2Pulse = 0, Channel3Pulse = 0, Channel4Pulse = 0;
u8  Channel1Base = 0,  Channel2Base = 0,  Channel3Base = 0,  Channel4Base = 0;

u16 CalcPuls(u8 val) {
  u16 output = (u16) (((u32) val * (TimerPeriod - 1)) / 256);
  return output;
}

void InitChannelPuls(void) {
  /* Compute the value to be set in ARR register to generate signal frequency at 17.57 Khz */
  TimerPeriod = (SystemCoreClock / 17570 ) - 1;
  Channel1Pulse = CalcPuls(0);
  Channel2Pulse = CalcPuls(0);
  Channel3Pulse = CalcPuls(0);
  Channel4Pulse = CalcPuls(0);
}

void AdjustChannelPuls(void) {
  Channel1Pulse = (Channel1Pulse <= 10)? TimerPeriod : Channel1Pulse - 10;
  Channel2Pulse = (Channel2Pulse <= 10)? TimerPeriod : Channel2Pulse - 10;
  Channel3Pulse = (Channel3Pulse <= 10)? TimerPeriod : Channel3Pulse - 10;
  Channel4Pulse = (Channel4Pulse <= 10)? TimerPeriod : Channel4Pulse - 10;
  TIM_ResetCounter(TIM3);
}

void UpdatePWM(void) {
  TIM_SetCompare1(TIM2, Channel1Pulse);
  TIM_SetCompare2(TIM2, Channel2Pulse);
  TIM_SetCompare3(TIM2, Channel3Pulse);
  TIM_SetCompare4(TIM2, Channel4Pulse);
}

/**
* Handle NRF24L01 Messages
*/
void EXTI15_10_IRQHandler(void) {
  LED_On();
  /* Make sure that interrupt flag is set */
  if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
    NRF24L01_IRQ_Handler(RX_BUF);

    printf("TP:%d, CH1:%d, CH2:%d, CH3:%d, CH4:%d\r\n", TimerPeriod, Channel1Pulse, Channel2Pulse, Channel3Pulse, Channel4Pulse);
    AdjustChannelPuls();
    UpdatePWM();
    
    /* Clear interrupt flag */
    EXTI_ClearITPendingBit(EXTI_Line13);
  }
  LED_Off();
}

void TIM_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOA, GPIOB Clocks enable */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB , ENABLE);

  /* GPIOA Configuration: Channel 1, 2, 3, 4 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);
}

void TIM_PWM_Init(void)
{
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* Time Base configuration */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  //TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_OCInitTypeDef  TIM_OCInitStructure;
  /* Channel 1, 2,3 and 4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // PWM: trigger from valid -> invalid
  // TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  //TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  // Specifies the TIM Output Compare pin state during Idle state, ## valid only for TIM1 and TIM8 ##
  //TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  //TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  //Specifies the TIM Output Compare state
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

  TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  TIM_OC1Init(TIM2, &TIM_OCInitStructure);
  // TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
  TIM_OC2Init(TIM2, &TIM_OCInitStructure);
  // TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
  TIM_OC3Init(TIM2, &TIM_OCInitStructure);
  // Enables or disables the TIMx peripheral Preload register on CCR1
  // TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_OCInitStructure.TIM_Pulse = Channel4Pulse;
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);
  // TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

  /* TIM2 counter enable */
  TIM_Cmd(TIM2, ENABLE);
}

/**
* Reset to initial state after 2 seconds
*/
void TIM3_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
    InitChannelPuls();
    UpdatePWM();
  }
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}


int main(void)
{
  Systick_Init();
  USART1_Init();
  // Timer
  TIM3_Init();
  TIM_Cmd(TIM3, ENABLE);

  // PWM
  InitChannelPuls();
  TIM_GPIO_Config();
  TIM_PWM_Init();
  UpdatePWM();

  // nRF24L01
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

