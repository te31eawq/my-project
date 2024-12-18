#include "LCD.h"

void LCDinit()
{
	Gpioinit();
	
	_delay_ms(15);
	// f = 0x38
	LCDwritecmddata(functionset); // function set = 0x38, 8bit
	_delay_ms(5);
	LCDwritecmddata(functionset);
	_delay_ms(1);
	LCDwritecmddata(functionset);
	LCDwritecmddata(functionset);
	LCDwritecmddata(displayoff);// 0b00001000; display off
	LCDwritecmddata(displayclear);// display clear
	LCDwritecmddata(entrymodeset);// (0b 000001 I/D S )entry mode set
	LCDwritecmddata(displayon);
}

void Gpioinit()
{
	//DDRB |= ((1<<5) | (1<<6) | (1<<7));
	Gpio_initpin(&DDRB, OUTPUT, 4);
	Gpio_initpin(&DDRB, OUTPUT, 6);
	Gpio_initpin(&DDRB, OUTPUT, 7);
	Gpio_initport(&DDRC, OUTPUT);
}

//MCU -> LCD로 명령어를 보낼 경우 (write mode)

void LCDcmdmode()
{
	//RS pin : bit 0 set
	//LCDcontrolport &= ~(1<<5);
	Gpio_writepin(&PORTB, 4, 0);
}
void LCDcharmode()
{
	// RS pin : bit 0 set
	// LCDcontrolport |= ~(1<<5);
	Gpio_writepin(&PORTB, 4, 1);
}
void LCDwritemode()
{
	// LCDcontrolport &= ~(1<<6);
	Gpio_writepin(&PORTB, 6, 0);
}
void LCDenablehigh()
{
	// e pin : 1세팅
	Gpio_writepin(&PORTB, 7, 1);
	_delay_ms(1);
}
void LCDenablelow()
{
	// e pin : 0세팅
	Gpio_writepin(&PORTB, 7, 0);
	_delay_ms(1);
}
void LCDwritebyte(uint8_t data)
{
	// 8 bit를 PORTC로 data 값을 내보내기
	Gpio_writeport(&PORTC, data);
}
void LCDwritecmddata(uint8_t data)
{
	//LCD 명령어 전달
	LCDcmdmode();
	LCDwritemode();
	LCDenablehigh();
	LCDwritebyte(data);
	LCDenablelow();
	
}
void LCDwritechardata(uint8_t data)
{
	//LCD character 쓰기 전달
	LCDcharmode();
	LCDwritemode();
	LCDenablehigh();
	LCDwritebyte(data);
	LCDenablelow();
}
void LCDwritestring(char *str)
{
	for(int i = 0 ; str[i]; i++)
	{
		LCDwritechardata(str[i]);
	}
}
void LCDgotoxy(uint8_t row, uint8_t col)
{
	col%= 16; row %=2;
	uint8_t lcdregisterAddress = (0x40 * row ) + col;
	uint8_t command = 0x80 + lcdregisterAddress;
	LCDwritecmddata(command);
} 
void LCDwrtiestringxy(uint8_t row, uint8_t col, char *str)
{
	LCDgotoxy(row,col);
	LCDwritestring(str);
}
void LCDcleardisplay()
{
	LCDwritecmddata(displayclear);
}