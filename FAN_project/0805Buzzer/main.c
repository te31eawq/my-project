#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ap/apwind.h"
#include "Periph/UART0/UART0.h"
#include <stdio.h>
#include <string.h>

FILE UARTMODE = FDEV_SETUP_STREAM(UART0_Transmit, NULL,_FDEV_SETUP_WRITE);

int main(void)
{
	wind_init();
	stdout = &UARTMODE;
	sei();
	while (1)
	{
		UART0_execute();
		wind_execute();
		
	}
}


