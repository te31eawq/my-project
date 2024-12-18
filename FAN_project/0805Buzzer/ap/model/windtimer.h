#ifndef WINDTIMER_H_
#define WINDTIMER_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

uint8_t model_getwindtimerdata();
void model_setwindtimerdata(uint8_t state);

#endif /* WINDTIMER_H_ */