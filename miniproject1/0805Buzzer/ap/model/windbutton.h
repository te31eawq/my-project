#ifndef WINDBUTTON_H_
#define WINDBUTTON_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

uint8_t model_getwindstatedata();
void model_setwindstatedata(uint8_t state);

#endif /* WINDBUTTON_H_ */