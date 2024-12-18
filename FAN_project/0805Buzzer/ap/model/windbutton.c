#include "windbutton.h"

uint8_t windstatedata;

uint8_t model_getwindstatedata()
{
	return windstatedata;
}

void model_setwindstatedata(uint8_t state)
{
	windstatedata = state;
}