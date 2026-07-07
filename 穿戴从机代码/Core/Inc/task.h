#ifndef _TASK_H_
#define _TASK_H_
#include "all_include.h"


extern unsigned int MUTEX_UART2;

extern UINT32 g_uart2_rx_sem;

extern uint8_t UART2_TEST_Flag;

void TASK_INIT(void);

void Task_Start(void);




#endif

