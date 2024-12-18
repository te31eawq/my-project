#include "Button.h"

void button_init(button_t *btn, volatile uint8_t *ddr, volatile uint8_t *pin, uint8_t pinNum)
{
	btn->DDR = ddr;
	btn->PIN = pin;
	btn->pinNum = pinNum;
	btn->prevState = 1;
	//*btn->DDR &= ~(1<<btn->pinNum);
	
	Gpio_initpin(btn->DDR, INPUT,btn->pinNum);
	
}

uint8_t Button_GetState(button_t *btn)
{
	//uint8_t curState = *btn->PIN & (1<<btn->pinNum);
	uint8_t curState = Gpio_readpin(btn->PIN, btn->pinNum);
	if((curState ==0) && (btn->prevState == 1))
	{
		_delay_ms(10);
		btn->prevState = 0; // PUSHED
		return 1; //ACT_PUSHED
	}
	else if((curState != 0) && (btn->prevState == 0))
	{
		_delay_ms(10);
		btn->prevState = 1; // RELEASED
		return 0; // ACT_RELEASED
	}
	return 2;
}
