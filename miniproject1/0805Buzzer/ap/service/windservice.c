#include "windservice.h"

uint16_t milisec3, sec3, min3, hour3;
uint16_t milisec, stsec, milisec2;
uint16_t sec, min, hour, sec2, min2, hour2;
uint8_t flagoff, timestart, showtime;
uint8_t prevsound;

void stopwatchinit()
{
	milisec = 0;
	sec = 0;
	min = 0;
	hour = 0;
	
	milisec2 = 0;
	sec2 = 0;
	min2 = 0;
	hour2 = 0;

}

void stopwatchincmilisec()
{
	uint8_t stopwatchstate;
	stopwatchstate = model_getwindautodata();
	if(stopwatchstate == 1)
	{
		milisec = (milisec + 1) % 1000;
	}
	
	if(milisec) return;
	
	sec = (sec+1) % 60;
	if(sec) return;
}
void timerincmilisec()
{
	uint8_t timerstate;
	uint8_t timersetting;
	timerstate = model_gettimestartdata();
	timersetting = model_getwindtimerdata();
	if(timerstate == 1 && timersetting >=1)
	{
		milisec2 = (milisec2 + 1) % 1000;
	}
	else return;
	
	if(milisec2) return;
	
	sec2 = (sec2+1) % 60;
	if(sec2) return;
	
	min2 = (min2 + 1) % 60;
	if(min2) return;
	
	hour2 = (hour2 + 1) % 24;
}

void setInitialTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint16_t milliseconds)
{
	hour3 = hours;
	min3 = minutes;
	sec3 = seconds;
	milisec3 = milliseconds;
}

void timerdecmilisec()
{
	uint8_t timerstate;
	uint8_t timersetting;
	timerstate = model_gettimestartdata();
	timersetting = model_getwindtimerdata();
	if(timerstate == 1 && timersetting >= 1)
	{
		if (milisec3 == 0)
		{
			milisec3 = 999;
			if (sec3 == 0)
			{
				sec3 = 59;
				if (min3 == 0)
				{
					min3 = 59;
					if (hour3 == 0)
					{
						// 타이머가 0에 도달
						hour3 = 0;
						min3 = 0;
						sec3 = 0;
						milisec3 = 0;
						return;
					}
					else
					{
						hour3--;
					}
				}
				else
				{
					min3--;
				}
			}
			else
			{
				sec3--;
			}
		}
		else
		{
			milisec3--;
		}
	}
	else
	{
		return;
	}
}

 void fndshow()
 {
	 showtime = model_getwindtimerdata();
	 timestart = model_gettimestartdata();
	 if(timestart == 1 && showtime == 0)
	 {
		 showtime = 1;
		 timestart = 0;
		 model_settimestartdata(timestart);
		 model_setwindtimerdata(showtime);
	 }
	 if(timestart == 1)
	 {
		 fndtimestart();
	 }
	 else if(timestart == 0)
	 {
		 fndtimerset();
	 }
 }

void clear()
{
	model_setwindtimerdata(0);
	model_settimestartdata(0);
	model_setwindstatedata(0);
	model_setwindautodata(0);
}

void stopreset()
{
	milisec2 = 0;
	sec2 = 0;
	min2 = 0;
	hour2 = 0;
}

void fndtimestart()
{
	switch(showtime)
	{
		case 1:
		clear();
		break;
		case 2:
		Presenter_dispstopwatchdata(3, min3, sec3, milisec3);
		if(min3 == 0 && sec3 == 0 && milisec3 == 0)
		{
			showtime = 0;
			model_setwindtimerdata(showtime);
			model_settimestartdata(0);
			buzzer2_timeover();
		}
		break;
		case 3:
		hour2 = 5;
		Presenter_dispstopwatchdata(5, min3, sec3, milisec3);
		if(min3 == 0 && sec3 == 0 && milisec3 == 0)
		{
			showtime = 0;
			model_setwindtimerdata(showtime);
			model_settimestartdata(0);
			buzzer2_timeover();
		}
		break;
		case 4:

		Presenter_dispstopwatchdata(7, min3, sec3, milisec3);
		if(min3 == 0 && sec3 == 0 && milisec3 == 0) // 취소하면 시간 설정 이어서 해야함
		{
			showtime = 0;
			model_setwindtimerdata(showtime);
			model_settimestartdata(0);
			buzzer2_timeover();
		}
		break;
	}
}

void fndtimerset()
{
	switch(showtime)
	{
		case 1:
		{
			prevsound = 0;
			showtime = 1;
			FND_setFndData(0000);
			model_setwindtimerdata(showtime);
			stopreset();
			clear();
		}
		break;
		case 2:
		{
			prevsound = 0;
			setInitialTime(0,3,0,0);
			Presenter_dispstopwatchdata2(0, 3, 0, 0);
			showtime = 2;
			model_setwindtimerdata(showtime);
			model_settimestartdata(0);

		}
		break;
		case 3:
		{
			prevsound = 0;
			setInitialTime(0,0,5,0); // 확인용으로 5초로 함
			Presenter_dispstopwatchdata2(0, 5, 0, 0);
			showtime = 3;
			model_setwindtimerdata(showtime);
			model_settimestartdata(0);

		}
		break;
		case 4:
		{
			prevsound = 0;
			setInitialTime(0,7,0,0);
			Presenter_dispstopwatchdata2(0, 7, 0, 0);
			showtime = 4;
			model_setwindtimerdata(showtime);
			model_settimestartdata(0);
		}
		break;
	}
}