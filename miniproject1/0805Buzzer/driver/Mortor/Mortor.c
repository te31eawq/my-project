#include "Mortor.h"

void Buzzer_soundOn()
{
	TCCR3A |= ((1<<COM3A1) | (0<<COM3A0));
}
void Buzzer_soundOff()
{
	TCCR3A &= ~((1<<COM3A1) | (1<<COM3A0));
}
void Buzzer_makehz(uint8_t strength, uint16_t hertz)
{
	if (hertz < 100) hertz = 100;
	else if(hertz > 5000) hertz = 5000;
	BUZZER_ICR = (250000/  hertz)-1;
	OCR3A = BUZZER_ICR / 4;				//Duty Cycle 50%
}
void Buzzer_init()
{
	TCCR3B |= (0<<CS32) | (1<<CS31) | (1<<CS30); // 주기값
	
	TCCR3B |= ((1<<WGM33) | (1<<WGM32));
	TCCR3A |= ((1<<WGM31) | (0<<WGM30));		// Fast PWM mode
	
	DDRE |= (1<<3);								// 핀 값 제공
}
void Buzzer_buttonOn(uint8_t strength, uint16_t hertz)
{
	Buzzer_makehz(strength, hertz);
	_delay_ms(10);
	TCNT3 = 0;
}