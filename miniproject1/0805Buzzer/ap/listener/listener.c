#include "listener.h"

button_t btnmode,btnauto,btntimer,timerstart;
uint8_t windstate,automode,timersel,startsw;

void listen_init()
{
	button_init(&btnmode, &DDRA, &PINA, 0);
	button_init(&btnauto, &DDRA, &PINA, 1);
	button_init(&btntimer, &DDRA, &PINA, 2);
	button_init(&timerstart, &DDRA, &PINA, 3);
	model_setwindstatedata(0);
	model_setwindautodata(0);
	model_setwindtimerdata(0);
	model_settimestartdata(0);
	automode = 0;
	windstate = 0;
	timersel = 1;
	startsw = 0;
}

void listen_execute()
{
	
	if(Button_GetState(&btntimer) == 1)
	{
		buzzer2_timechange();
		timersel++;
		model_setwindtimerdata(timersel);
		windstate = 0;
		model_setwindstatedata(windstate);
		automode = 0;
		model_setwindautodata(automode);
		startsw = 0;
		model_settimestartdata(startsw);
		if(timersel == 5)
		{
			timersel = 1;
			model_setwindtimerdata(timersel);
		}
	}
	
	if(Button_GetState(&timerstart) == 1)
	{
		buzzer2_timestart();
		startsw++;
		model_settimestartdata(startsw);
		if(startsw >= 2)
		{
			startsw = 0;
			model_settimestartdata(startsw);
		}
	}
}

void listener_state()
{
	if(Button_GetState(&btnmode) == 1)
	{
		buzzer2_windchange();
		windstate++;
		model_setwindstatedata(windstate);
		if(windstate >= 4)
		{
			buzzer2_windoff();
			windstate = 0;
			model_setwindstatedata(windstate);
		}
		model_setwindautodata(0);
		model_settimestartdata(0);
	}
	if(Button_GetState(&btnauto) == 1)
	{
		buzzer2_timestart();
		model_setwindautodata(1);
		model_setwindstatedata(windstate);
	}
}