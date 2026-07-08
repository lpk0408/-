#include "all.h"
#include "BEEP.h"

/*蜂鸣器*/
unsigned int beep_duty = 10;
unsigned int beep_ret = 0;

void BEEP_Init(void)
{
    beep_ret = IoTPwmInit(BEEP_PORT);
    if (beep_ret != 0) {
        printf("IoTPwmInit failed(%d)\n", BEEP_PORT);
    }
    
    printf("PWM(%d) Start\n", BEEP_PORT);
    printf("duty: %d\r\n", beep_duty);
}

void BEEP_ENABLE(void)
{   
    beep_ret = IoTPwmStart(BEEP_PORT, beep_duty, 1000);
    if (beep_ret != 0)
    {
        printf("IoTPwmStart failed(%d)\n");
    }
}

void BEEP_DISABLE(void)
{
    beep_ret = IoTPwmStop(BEEP_PORT);
    if (beep_ret != 0)
    {
        printf("IoTPwmStop failed(%d)\n");
    }
}


