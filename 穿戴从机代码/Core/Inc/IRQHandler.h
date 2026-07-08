#ifndef _IRQHANDLER_H_
#define	_IRQHANDLER_H_

#include "all_include.h"

#define CHAR_SEND_BUF_SIZE	128

#define HEX_SEND_BUF_SIZE		128 * 2

#define IRQHANDLER_UART2_RX_BUF_SIZE  128

#define IRQHANDLER_UART2_RX_RING_BUF_SIZE  1024

//extern uint8_t IRQHandler_UART2_Rev_Temp[HEX_SEND_BUF_SIZE];

//extern char IRQHandler_UART2_Send_buf[CHAR_SEND_BUF_SIZE];

//extern volatile uint32_t IRQHandler_UART2_rev_p;

//extern volatile uint8_t IRQHandler_UART2_rev_final_Flag;

//extern uint8_t IRQHandler_UART2_Rev_rx;

//-----uart1----
extern uint8_t IRQHandler_UART3_Rev_rx;

extern volatile uint32_t IRQHandler_UART3_rev_p;

extern volatile uint8_t IRQHandler_UART3_rev_final_Flag;

extern uint8_t IRQHandler_UART3_Rev_Temp[HEX_SEND_BUF_SIZE];

//-----uart2-----
extern uint8_t IRQHandler_Uart2_rx_buf1[IRQHANDLER_UART2_RX_BUF_SIZE];

extern uint8_t IRQHandler_Uart2_rx_buf2[IRQHANDLER_UART2_RX_BUF_SIZE];

extern volatile uint8_t IRQHandler_Uart2_rx_complete_flag;

extern uint8_t *IRQHandler_Uart2_current_rx_buf;

extern uint8_t IRQHandler_Uart2_rx_ring_buf[IRQHANDLER_UART2_RX_RING_BUF_SIZE];

extern volatile uint32_t IRQHandler_uart2_ring_wr;

extern uint32_t IRQHandler_uart2_ring_rd;

extern volatile uint32_t debug_rx_event_cnt;

//-----回调函数----
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

#endif


