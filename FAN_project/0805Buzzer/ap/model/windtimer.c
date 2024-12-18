#include "windtimer.h"

uint8_t windtimerdata;

uint8_t model_getwindtimerdata()
{
	return windtimerdata;
}

void model_setwindtimerdata(uint8_t state)
{
	windtimerdata = state;
}