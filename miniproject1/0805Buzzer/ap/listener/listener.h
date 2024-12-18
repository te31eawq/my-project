#ifndef LISTENER_H_
#define LISTENER_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../../driver/button/Button.h"
#include "../model/windbutton.h"
#include "../model/windauto.h"
#include "../model/windtimer.h"
#include "../model/timerstart.h"
#include "../../driver/buzzer/buzzer.h"

void listen_init();
void listen_execute();
void listener_state();

#endif /* LISTENER_H_ */