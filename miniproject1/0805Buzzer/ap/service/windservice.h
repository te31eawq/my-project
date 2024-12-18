#ifndef WINDSERVICE_H_
#define WINDSERVICE_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../model/windbutton.h"
#include "../model/windauto.h"
#include "../model/modelwindnext.h"
#include "../model/windtimer.h"
#include "../model/timerstart.h"
#include "../presenter/presenter.h"
#include "../../driver/buzzer/buzzer.h"
#include "../../driver/Mortor/Mortor.h"

void stopwatchinit();
void stopwatchincmilisec();
void timerincmilisec();
void fndshow();
void stopreset();
void clear();
void timerdecmilisec();
void setInitialTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint16_t milliseconds);
void fndtimestart();
void fndtimerset();

#endif /* WINDSERVICE_H_ */