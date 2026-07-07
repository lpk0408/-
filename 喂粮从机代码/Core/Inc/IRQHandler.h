#ifndef _IRQHANDLER_H_
#define	_IRQHANDLER_H_

#include "all_include.h"

#define CHAR_SEND_BUF_SIZE	128

#define HEX_SEND_BUF_SIZE		128 * 2

extern uint8_t IRQHandler_UART2_Rev_Temp[HEX_SEND_BUF_SIZE];

extern uint8_t IRQHandler_UART3_Rev_Temp[HEX_SEND_BUF_SIZE];

extern char IRQHandler_UART2_Send_buf[CHAR_SEND_BUF_SIZE];

extern volatile uint32_t IRQHandler_UART2_rev_p;

extern volatile uint8_t IRQHandler_UART2_rev_final_Flag;

extern uint8_t IRQHandler_UART3_Rev_rx;

extern uint8_t IRQHandler_UART2_Rev_rx;

extern volatile uint32_t IRQHandler_UART3_rev_p;

extern volatile uint8_t IRQHandler_UART3_rev_final_Flag;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif


