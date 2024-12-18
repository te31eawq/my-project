#include "windauto.h"

uint8_t autostatedata;

uint8_t model_getwindautodata()
{
	return autostatedata;
}

void model_setwindautodata(uint8_t state)
{
	autostatedata = state;
}