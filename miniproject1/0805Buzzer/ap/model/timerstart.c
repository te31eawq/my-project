#include "timerstart.h"

uint8_t timestartdata;

uint8_t model_gettimestartdata()
{
	return timestartdata;
}

void model_settimestartdata(uint8_t state)
{
	timestartdata = state;
}