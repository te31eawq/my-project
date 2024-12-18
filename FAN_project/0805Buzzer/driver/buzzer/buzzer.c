#include "buzzer.h"


void Buzzer2_soundOn()
{
	TCCR1A |= ((1<<COM1A1) | (0<<COM1A0));
}
void Buzzer2_soundOff()
{
	TCCR1A &= ~((1<<COM1A1) | (1<<COM1A0));
}
void Buzzer2_makehz(uint8_t strength, uint16_t hertz)
{
	if (hertz < 100) hertz = 100;
	else if(hertz > 5000) hertz = 5000;
	BUZZER2_ICR = (250000/  hertz)-1;
	BUZZER2_OCR = BUZZER2_ICR / 4;				//Duty Cycle 50%
}
void Buzzer2_init()
{
	TCCR1B |= (0<<CS12) | (1<<CS11) | (1<<CS10); // 주기값
	
	TCCR1B |= ((1<<WGM13) | (1<<WGM12));
	TCCR1A |= ((1<<WGM11) | (0<<WGM10));		// Fast PWM mode
	
	DDRB |= (1<<5);					// 핀 값 제공
}
void Buzzer2_buttonOn(uint8_t strength, uint16_t hertz)
{
	Buzzer_makehz(strength, hertz);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer_makehz(strength, hertz);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer_makehz(strength, hertz);
	_delay_ms(100);
	TCNT1 = 0;
}


void buzzer2_buttonset()
{
	Buzzer2_makehz(4,600);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_makehz(4,800);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_makehz(4,900);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_makehz(4, 1000);
	_delay_ms(100);
	TCNT1 = 0;
}

void buzzer2_windchange()
{
	Buzzer2_soundOn();
	Buzzer2_makehz(4,600);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_soundOff();
}

void buzzer2_windoff()
{
	Buzzer2_soundOn();
	Buzzer2_makehz(4,400);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_soundOff();
}


void buzzer2_automode()
{
	Buzzer2_soundOn();
	Buzzer2_makehz(4,800);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_makehz(4,600);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_makehz(4,900);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_soundOff();
}

void buzzer2_timechange()
{
	Buzzer2_soundOn();
	Buzzer2_makehz(4,800);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_soundOff();
}

void buzzer2_timestart()
{
	Buzzer2_soundOn();
	Buzzer2_makehz(4,550);
	_delay_ms(100);
	TCNT1 = 0;
	Buzzer2_soundOff();
}

void buzzer2_timeover()
{
	Buzzer2_soundOn();
	Buzzer2_makehz(4,800);
	_delay_ms(200);
	TCNT1 = 0;
	Buzzer2_makehz(4,600);
	_delay_ms(200);
	TCNT1 = 0;
	Buzzer2_makehz(4,500);
	Buzzer2_soundOff();
}