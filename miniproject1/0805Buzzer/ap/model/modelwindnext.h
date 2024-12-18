#ifndef MODELWINDNEXT_H_
#define MODELWINDNEXT_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

uint8_t model_getwindnextdata();
void model_setwindnextdata(uint8_t state);

#endif /* MODELWINDNEXT_H_ */