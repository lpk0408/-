#define SG_PORT EPWMDEV_PWM7_M1

void SG_Init(void);

void SG_Init_ENABLE(void);

void SG_Init_DISABLE(void);

void SG_SET_Angle(unsigned int angle);

extern unsigned int sg_duty;

void dat_turnon(void);

void dat_turnoff(void);
