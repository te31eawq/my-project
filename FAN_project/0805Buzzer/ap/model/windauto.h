#ifndef WINDAUTO_H_
#define WINDAUTO_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

uint8_t model_getwindautodata();

void model_setwindautodata(uint8_t state);

#endif /* WINDAUTO_H_ */