#include "UART0.h"

uint8_t uart0Rxflag;
uint8_t uart0Rxbuff[100];


// LED1_TOGGLE\n
// LED2_TOGGLE\n
// LED3_TOGGLE\n
void UART0_ISR_Process()
{
	static uint8_t uart0RxTail = 0;
	uint8_t rx0Data = UDR0;

	if(rx0Data == '\n')
	{
		uart0Rxbuff[uart0RxTail] = rx0Data;
		uart0RxTail++;
		uart0Rxbuff[uart0RxTail] = 0;
		uart0RxTail = 0;
		UART0_setRxFlag();
	}
	else
	{
		uart0Rxbuff[uart0RxTail] = rx0Data;
		uart0RxTail++;
	}
}

void UART0_Transmit(unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

unsigned char UART0_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}

void UART0_sendString(char *str)
{
	for(int i=0;str[i];i++)
	{
		UART0_Transmit(str[i]);
	}
}

uint8_t UART0_Avail()
{
	if( !(UCSR0A & (1<<RXC0)) )
	{
		return 0; // Rx Data가 없으면 0
	}
	return 1; // 있으면 1
}

void UART0_init()
{
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0); //UCSR1B, UCSR0B 중에 선택한거임
	UCSR0A |= (1<<U2X0);//2배 모드
	
	UBRR0L = 207; // 16mhz
	UCSR0B |= (1<<RXCIE0); // 1byte 수신
}

void UART_execute()
{
	UART0_sendString("TEST\n");
	if(UART0_GetRxFlag())
	{
		UART0_clearRxFlag();
		UART0_sendString("Receive Data : ");
		UART0_sendString((char *)UART0_readRxBuff());
	}
	_delay_ms(1000);
}

void UART0_clearRxFlag()
{
	uart0Rxflag = 0;
}


void UART0_setRxFlag()
{
	uart0Rxflag = 1;
}

uint8_t UART0_GetRxFlag()
{
	return uart0Rxflag;
}

uint8_t *UART0_readRxBuff()
{
	return uart0Rxbuff;
}