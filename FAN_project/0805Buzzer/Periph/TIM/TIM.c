#include "TIM.h"

void TIM0init() // 1ms overflow Interrupt, FND Display 용도 
{
	TCCR0 |= ((1<<CS02) | (0<<CS01) | (1<<CS00));	// 128 PreScaler 설정
	TIMSK |= (1<<TOIE0);							// TIMSK Timer/Counter0 overflow Interrupt Enable
	TCNT0 = 130;									// 130부터 시작하라고 선언
}

void TIM2init()
{
	// Timer/Counter 2 1/64 1ms period interrupt enable
	TCCR2 |= ((0<<CS22) | (1<<CS21) | (1<<CS20));	// 128 PreScaler 설정
	TIMSK |= (1<<OCIE2);
	TCCR2 |= ((1<<WGM21) | (0<<WGM20));
	OCR2 = 250-1;		// TIMSK Timer/Counter0 overflow Interrupt Enable
}