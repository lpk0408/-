#ifndef _SU_03T_H_
#define _SU_03T_H_


enum light_command
{
    light_state_on = 0x0101,
    light_state_off,
};

enum temperature_command
{
    temperature_get = 0x0103,
    humidity_get,
    illumination_get,
};

void su_03t_thread();

#endif
