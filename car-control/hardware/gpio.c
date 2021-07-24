#include "gpio.h"

void Project_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TM_GPIO_Mode_t GPIO_Mode, TM_GPIO_OType_t GPIO_OType, TM_GPIO_PuPd_t GPIO_PuPd, TM_GPIO_Speed_t GPIO_Speed) {	
	/* Check input */
	if (GPIO_Pin == 0x00) {
		return;
	}
	
	/* Enable clock for GPIO */
	TM_GPIO_INT_EnableClock(GPIOx);
	
	/* Do initialization */
	TM_GPIO_INT_Init(GPIOx, GPIO_Pin, GPIO_Mode, GPIO_OType, GPIO_PuPd, GPIO_Speed);
}