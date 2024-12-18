#ifndef TIMER0_H_
#define TIMER0_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void TIM0init();
void TIM2init();
#endif /* TIMER0_H_ */