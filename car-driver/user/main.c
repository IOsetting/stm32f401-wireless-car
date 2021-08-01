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

/*
TargetFrequency = 50
SystemCoreClock = 84MHz = 84,000,000

so
TotalDivider = SystemCoreClock/TargetFrequency = 1,680,000

so
TIM_Prescaler = 1,680,000 / 65535 - 1 = 25.6 - 1 = 25, make it 29

so 
ScaledFrequency = 84,000,000 / (29 + 1) = 2,800,000

because
PWM_frequency = timer_tick_frequency / (TIM_Period + 1)
so
50 = 2,800,000 / (TIM_Period + 1),
(TIM_Period + 1) = 2,800,000/50 = 56,000,
TIM_Period = 55999
*/
u16 TimerPeriod = 55999;
u32 Channel1Pulse = 0, Channel2Pulse = 0, Channel3Pulse = 0, Channel4Pulse = 0;
u8  center_d = 5;

u32 CalcPuls(u8 val)
{
  u32 output = ((u32) val * TimerPeriod) / 127;
  return output;
}

void InitChannelPuls(void) 
{
  Channel1Pulse = 0;
  Channel2Pulse = 0;
  Channel3Pulse = 0;
  Channel4Pulse = 0;
  TIM_SetCompare1(TIM2, Channel1Pulse);
  TIM_SetCompare2(TIM2, Channel2Pulse);
  TIM_SetCompare3(TIM2, Channel3Pulse);
  TIM_SetCompare4(TIM2, Channel4Pulse);
}

void XY2LR(u8 inx, u8 iny, int8_t *l, int8_t *r)
{
  // int8_t only support [-128, 127]
  if (inx == 255) inx = 254;
  if (iny == 255) iny = 254;
  // Move point to center
  int8_t axisx = inx - 127;
  int8_t axisy = iny - 127;
  printf("centered: X:%d, Y:%d\r\n", axisx, axisy);
  // To different quadrants
  if (axisx >= 0 && axisy >= 0) { // Quadrant 1
    printf("Quadrant 1\r\n");
    if (axisx < center_d && axisy < center_d) {
      printf("Ignored\r\n");
      *l = 0; *r = 0;
      return;
    }
    if (axisx > axisy) { // 0 ~ 45
      int8_t vect = axisx;
      *l = vect;
      *r = (int8_t)((int16_t) (vect * axisy) / axisx - vect);
    } else {
      int8_t vect = axisy;
      *l = vect;
      *r = (int8_t)(vect - (int16_t)(vect * axisx) / axisy);
    }
    
  } else if (axisx < 0 && axisy >= 0) { // Quadrant 2
    printf("Quadrant 2\r\n");
    if (axisx > -center_d && axisy < center_d) {
      printf("Ignored\r\n");
      *l = 0; *r = 0;
      return;
    }
    axisx = -axisx; // Change to absolute value
    if (axisx > axisy) {
      int8_t vect = axisx;
      *l = (int8_t)((int16_t)(vect * axisy) / axisx - vect);
      *r = vect;

    } else {
      int8_t vect = axisy;
      *l = (int8_t)(vect - (int16_t)(vect * axisx) / axisy);
      *r = vect;

    }
    
  } else if (axisx < 0 && axisy < 0) { // Quadrant 3
    printf("Quadrant 3\r\n");
    if (axisx > -center_d && axisy > -center_d) {
      printf("Ignored\r\n");
      *l = 0; *r = 0;
      return;
    }
    axisx = -axisx; // Change to absolute value
    axisy = -axisy; // Change to absolute value
    if (axisx > axisy) {
      int8_t vect = axisx;
      *l = -vect;
      *r = (int8_t)(vect - (int16_t)(vect * axisy) / axisx);

    } else {
      int8_t vect = axisy;
      *l = -vect;
      *r = (int8_t)((int16_t)(vect * axisx) / axisy - vect);

    }

  } else { // Quadrant 4
    printf("Quadrant 4\r\n");
    if (axisx < center_d && axisy > -center_d) {
      printf("Ignored\r\n");
      *l = 0; *r = 0;
      return;
    }
    axisy = -axisy; // Change to absolute value
    printf("axisx: %d, axisy: %d\r\n", axisx, axisy);
    if (axisx > axisy) {
      int8_t vect = axisx;
      *l = (int8_t)(vect - (int16_t)(vect * axisy) / axisx);
      *r = -vect;
      printf("0 vect: %d, *r: %d\r\n", vect, *r);
    } else {
      int8_t vect = axisy;
      *l = (int8_t)((int16_t)(vect * axisx) / axisy - vect);
      *r = -vect;
      printf("1 vect: %d, *r: %d\r\n", vect, *r);
    }

  }
}

/*
双轴摇杆: PIN脚朝左, X轴左小右大, Y轴上小下大
前进: X中,Y小
后退: X中,Y大
左转: X小,Y中
右转: X大,Y中
*/
void AdjustChannelPuls(u8 axis_x, u8 axis_y)
{
  int8_t l, r;
  // Revert Y value to standard XY directions
  axis_y = 255 - axis_y;
  // Transform XY to LR
  XY2LR(axis_x, axis_y, &l, &r);
  printf("X:%d, Y:%d, L:%d, R:%d\r\n", axis_x, axis_y, l, r);
  if (l >= 0) {
    Channel1Pulse = CalcPuls(l);
    Channel3Pulse = 0;
    // Set the PWM:0 channel first
    TIM_SetCompare3(TIM2, Channel3Pulse);
  } else {
    Channel1Pulse = 0;
    Channel3Pulse = CalcPuls(-l);
    TIM_SetCompare1(TIM2, Channel1Pulse);
  }
  if (r >= 0) {
    Channel2Pulse = CalcPuls(r);
    Channel4Pulse = 0;
    TIM_SetCompare4(TIM2, Channel4Pulse);
  } else {
    Channel2Pulse = 0;
    Channel4Pulse = CalcPuls(-r);
    TIM_SetCompare2(TIM2, Channel2Pulse);
  }

  if (l >= 0) {
    TIM_SetCompare1(TIM2, Channel1Pulse);
  } else {
    TIM_SetCompare3(TIM2, Channel3Pulse);
  }
  if (r >= 0) {
    TIM_SetCompare2(TIM2, Channel2Pulse);
  } else {
    TIM_SetCompare4(TIM2, Channel4Pulse);
  }

  TIM_ResetCounter(TIM3);
}


/**
* Handle NRF24L01 Messages
*/
void EXTI15_10_IRQHandler(void) 
{
  LED_On();
  /* Make sure that interrupt flag is set */
  if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
    NRF24L01_IRQ_Handler(RX_BUF);
    if (RX_BUF[0] == 0x02) { // Ensure it is a valid input
      AdjustChannelPuls(RX_BUF[1], RX_BUF[2]);
      printf("TP:%d, CH1/CH3:%d/%d, CH2/CH4:%d/%d\r\n", TimerPeriod, Channel1Pulse, Channel3Pulse, Channel2Pulse, Channel4Pulse);
    }
    /* Clear interrupt flag */
    EXTI_ClearITPendingBit(EXTI_Line13);
  }
  LED_Off();
}

/**
* Reset to initial state after certain period
*/
void TIM3_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
    InitChannelPuls();
  }
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}


void TIM_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOA, GPIOB Clocks enable */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB , ENABLE);

  /* GPIOA Configuration: Channel 1, 2, 3, 4 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
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
  /*
    TIM2 is connected to APB1 bus, which has on F401 device 42MHz clock, but, timer has internal PLL, which double this frequency for timer, up to 84MHz.
    Remember: Not each timer is connected to APB1, there are also timers connected on APB2, which works at 84MHz by default, and internal PLL increase this to up to 168MHz.

    Set timer prescaller
    Timer count frequency is set with

    timer_tick_frequency = Timer_default_frequency / (prescaller_set + 1)

    In our case, we want a max frequency for timer, so we set prescaller to 0
    And our timer will have tick frequency

    timer_tick_frequency = 84000000 / (0 + 1) = 84000000
  */
  TIM_TimeBaseStructure.TIM_Prescaler = 29;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  /*
    Set timer period when it have reset
    First you have to know max value for timer
    In our case it is 16bit = 65535
    To get your frequency for PWM, equation is simple

    PWM_frequency = timer_tick_frequency / (TIM_Period + 1)

    If you know your PWM frequency you want to have timer period set correct

    TIM_Period = timer_tick_frequency / PWM_frequency - 1

    In our case, for 10Khz PWM_frequency, set Period to

    TIM_Period = 84000000 / 10000 - 1 = 8399

    If you get TIM_Period larger than max timer value (in our case 65535),
    you have to choose larger prescaler and slow down timer tick frequency
  */
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  //TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_OCInitTypeDef  TIM_OCInitStructure;
  /* Channel 1, 2,3 and 4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // PWM: trigger from valid -> invalid
  // TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
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

  TIM_OCInitStructure.TIM_Pulse = Channel4Pulse;
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);

  /* TIM2 counter enable */
  TIM_Cmd(TIM2, ENABLE);
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

