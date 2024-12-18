#ifndef TIMERSTART_H_
#define TIMERSTART_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

uint8_t model_gettimestartdata();
void model_settimestartdata(uint8_t state);


#endif /* TIMERSTART_H_ */