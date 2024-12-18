#ifndef APWIND_H_
#define APWIND_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "../driver/buzzer/buzzer.h"
#include "../driver/button/Button.h"
#include "model/windbutton.h"
#include "model/windauto.h"
#include "service/windservice.h"
#include "listener/listener.h"
#include "../driver/FND/fnd.h"
#include "../driver/LCD/LCD.h"
#include "presenter/presenter.h"
#include "../Periph/UART0/UART0.h"

void wind_init();
void wind_execute();
void windauto();
void windhand();
void windsetting();
void UART0_execute();
void buttoncheck();

#endif /* WIND_H_ */