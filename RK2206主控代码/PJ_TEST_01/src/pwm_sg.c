#include "all.h"
#include "pwm_sg.h"

unsigned int sg_duty = 30;
unsigned int sg_ret = 0;
unsigned int freq = 50;

void SG_Init(void)
{
    sg_ret = IoTPwmInit(SG_PORT);
    if (sg_ret != 0)
    {
        printf("IoTPwmInit failed(%d)\n", SG_PORT);
    }
    
    printf("PWM(%d) Start\n", SG_PORT);
    printf("duty: %d\r\n", sg_duty);
}

void SG_Init_ENABLE(void)
{   
    sg_ret = IoTPwmStart(SG_PORT, sg_duty, freq);
    if (sg_ret != 0)
    {
        printf("IoTPwmStart failed(%d)\n",sg_ret);
    }
}

void SG_Init_DISABLE(void)
{
    sg_ret = IoTPwmStop(SG_PORT);
    if (sg_ret != 0)
    {
        printf("IoTPwmStart failed(%d)\n",sg_ret);
    }
}

void SG_SET_Angle(unsigned int angle)
{   
    if(angle >= 180) angle = 180;

    float duty_f = 2.5 + (angle / 180.0f) * 10.0f;

    unsigned int CCR = (unsigned int)(duty_f + 0.5);

    sg_ret = IoTPwmStart(SG_PORT, CCR, freq);
    if (sg_ret != 0)
    {
        printf("IoTPwmStart failed(%d)\n",sg_ret);
    }

}


void dat_turnon(void)
{
    SG_SET_Angle(0);
}

void dat_turnoff(void)
{
    SG_SET_Angle(90);
}

