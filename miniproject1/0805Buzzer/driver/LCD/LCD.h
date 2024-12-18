#ifndef LCD_H_
#define LCD_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "../../Periph/GPIO/gpio.h"

#define LCDcontrolport PORTB
#define LCDrs 5
#define LCDcontrolddr DDRB
#define LCDdataddr DDRC
#define functionset 0x38

#define displayoff 0x08
#define displayclear 0x01
#define entrymodeset 0x06
#define displayon 0x0c

void Gpioinit();
void LCDwritecmddata(uint8_t data);

void LCDinit();
void Gpioinit();
void LCDcmdmode();
void LCDcharmode();
void LCDwritemode();
void LCDenablehigh();
void LCDenablelow();
void LCDwritebyte(uint8_t data);
void LCDwritecmddata(uint8_t data);
void LCDwritechardata(uint8_t data);
void LCDwritestring(char *str);
void LCDgotoxy(uint8_t row, uint8_t col);
void LCDwrtiestringxy(uint8_t row, uint8_t col, char *str);
void LCDcleardisplay();




#endif /* LCD_H_ */