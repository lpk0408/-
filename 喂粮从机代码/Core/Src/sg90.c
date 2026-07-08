#include "sg90.h"
#include "tim.h"


uint16_t angle = 0;

void SG_SET_Angle(uint16_t angle)
{
	if(angle > 180) angle = 180;
	if(angle < 0) angle = 0;
	
	float duty_f = 2.5 + (angle / 180.0f) * 10.0f;

	unsigned int CCR = (unsigned int)(duty_f + 0.5);
	
	TIM12->CCR1 = CCR;
}

