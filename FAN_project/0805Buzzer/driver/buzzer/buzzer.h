#ifndef BUZZER_H_
#define BUZZER_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#define BUZZER_ICR ICR3
#define BUZZER_OCR OCR3A
#define BUZZER2_ICR ICR1
#define BUZZER2_OCR OCR1A

void Buzzer_soundOn();
void Buzzer_soundOff();
void Buzzer_makehz(uint8_t strength, uint16_t hertz);
void Buzzer_init();
void Buzzer_buttonOn(uint8_t strength, uint16_t hertz);

void Buzzer2_soundOn();
void Buzzer2_soundOff();
void Buzzer2_makehz(uint8_t strength, uint16_t hertz);
void Buzzer2_init();
void Buzzer2_buttonOn(uint8_t strength, uint16_t hertz);
void buzzer2_buttonset();

void buzzer2_windchange();
void buzzer2_windoff();
void buzzer2_automode();
void buzzer2_timechange();
void buzzer2_timestart();
void buzzer2_timeover();
#endif /* BUZZER_H_ */