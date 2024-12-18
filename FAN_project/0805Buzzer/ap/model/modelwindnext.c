#include "modelwindnext.h"

uint8_t windnextstatedata;

uint8_t model_getwindnextdata()
{
	return windnextstatedata;
}

void model_setwindnextdata(uint8_t state)
{
	windnextstatedata = state;
}