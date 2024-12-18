#include "presenter.h"

void presenterinit()
{
	fndinit();
	LCDinit();
}

void timezerover(uint8_t hour,uint8_t min,uint8_t sec,uint16_t milisec)
{
	uint16_t stopwatch2data;
	
	stopwatch2data = (min*100)+(sec);
	FND_setFndData(stopwatch2data);
	
	LCDwrtiestringxy(0,3,"TIME SETTING");
	LCDwrtiestringxy(1,3," No Time   ");
	
}


void timezerofnd(uint8_t hour,uint8_t min,uint8_t sec,uint16_t milisec)
{
	uint16_t stopwatch2data;
	
	stopwatch2data = (min*100)+(sec);
	FND_setFndData(stopwatch2data);
}

void showstrength(uint8_t strength)
{
	uint8_t charge;
	charge = strength;
	char buff[30];
	LCDwrtiestringxy(0,3,"TIME STATE");
	switch(charge)
	{
		case 0:
		sprintf(buff, "STATE : OFF ");
		LCDwrtiestringxy(1,3,buff);
		break;
		case 1:
		sprintf(buff, "STATE : 1  ");
		LCDwrtiestringxy(1,3,buff);
		break;
		case 2:
		sprintf(buff, "STATE : 2   ");
		LCDwrtiestringxy(1,3,buff);
		break;
		case 3:
		sprintf(buff, "STATE : 3   ");
		LCDwrtiestringxy(1,3,buff);
		break;
	}
}

void Presenter_dispstopwatchdata(uint8_t hour,uint8_t min,uint8_t sec,uint16_t milisec)
{
	uint16_t stopwatch2data;
	
	stopwatch2data = (min*100)+(sec);
	FND_setFndData(stopwatch2data);
	
	char buff[30];
	sprintf(buff, " %2d MINUTE ",hour);
	LCDwrtiestringxy(0,2,buff);
	sprintf(buff, " %02d:%02d:%02d   ", min, sec, milisec/10%100);
	LCDwrtiestringxy(1,3,buff);
	
}

void fndfan(uint16_t dir)
{
	uint16_t fndfandata;
	
	fndfandata = dir;
	FND_dispfan(fndfandata);
}

void Presenter_dispstopwatchdata2(uint8_t hour,uint8_t min,uint8_t sec,uint16_t milisec)
{
	uint16_t stopwatch2data;
	
	stopwatch2data = (min*100)+(sec);
	FND_setFndData(stopwatch2data);
	
	char buff[30];
	sprintf(buff, " %2d MINUTE ",min);
	LCDwrtiestringxy(0,2,buff);
	sprintf(buff, " %02d:%02d:%02d   ", min, sec, milisec/10%100);
	LCDwrtiestringxy(1,3,buff);
	
}