#include "gpio.h"

// DDR 1. port 2. pin

void Gpio_initport(volatile uint8_t* DDR, uint8_t dir) // 포트 인풋 정하기
{
	if(dir == OUTPUT)
	{
		*DDR = 0xff;
	}
	else
	{
		*DDR = 0x00;
	}
}
void Gpio_initpin(volatile uint8_t *DDR, uint8_t dir, uint8_t pinNum) // 특정 핀 인풋정하기
{
	if (dir == OUTPUT)
	{
		*DDR |= (1<<pinNum);
	}
	else
	{
		*DDR &= ~(1<<pinNum);
	}
}
void Gpio_writeport(volatile uint8_t* PORT, uint8_t data) //포트에서 값 그대로 대입하기
{
	*PORT = data;
}
void Gpio_writepin(volatile uint8_t* PORT, uint8_t pinNum, uint8_t state) // 포트에서 특정 핀 상태 정하기
{
	if(state == GPIO_PIN_SET)
	{
		*PORT |= (1<<pinNum);
	}
	else
	{
		*PORT &= ~(1<<pinNum);
	}
}
uint8_t Gpio_readport(volatile uint8_t* PIN) // 핀 그대로 반환 말 그대로 읽기
{
	return *PIN;
}
uint8_t Gpio_readpin(volatile uint8_t* PIN, uint8_t pinNum) // 특정 핀 읽기
{
	return ((*PIN & (1<<pinNum)) != 0);
}
