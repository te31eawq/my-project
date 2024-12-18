#ifndef PRESENTER_H_
#define PRESENTER_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "../../driver/LCD/LCD.h"
#include "../../driver/FND/fnd.h"
#include "../model/windtimer.h"

void presenterinit();
void Presenter_dispstopwatchdata(uint8_t hour,uint8_t min,uint8_t sec,uint16_t milisec);
void Presenter_dispstopwatchdata2(uint8_t hour2,uint8_t min2,uint8_t sec2,uint16_t milisec2);
void timezerover(uint8_t hour,uint8_t min,uint8_t sec,uint16_t milisec);
void showstrength(uint8_t strength);
void timezerofnd(uint8_t hour,uint8_t min,uint8_t sec,uint16_t milisec);
void fndfan(uint16_t dir);


#endif /* PRESENTER_H_ */