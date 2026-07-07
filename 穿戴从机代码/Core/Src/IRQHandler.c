#include "IRQHandler.h"
#include "task.h"

//-----信号量

extern UINT32 g_uart2_rx_sem;

//-----UART 3-----
uint8_t IRQHandler_UART3_Rev_Temp[HEX_SEND_BUF_SIZE];

uint8_t IRQHandler_UART3_Rev_rx;

volatile uint32_t IRQHandler_UART3_rev_p;

volatile uint8_t IRQHandler_UART3_rev_final_Flag;

//-----UART2 DMA RX-------

uint8_t IRQHandler_Uart2_rx_buf1[IRQHANDLER_UART2_RX_BUF_SIZE];

uint8_t IRQHandler_Uart2_rx_buf2[IRQHANDLER_UART2_RX_BUF_SIZE];

uint8_t *IRQHandler_Uart2_current_rx_buf = IRQHandler_Uart2_rx_buf1;

volatile uint8_t IRQHandler_Uart2_rx_complete_flag = 0;   // 接收完成标志

volatile uint16_t IRQHandler_Uart2_rx_data_len = 0;       // 实际接收到的数据长度

uint8_t IRQHandler_Uart2_rx_ring_buf[IRQHANDLER_UART2_RX_RING_BUF_SIZE];

volatile uint32_t IRQHandler_uart2_ring_wr = 0;

uint32_t IRQHandler_uart2_ring_rd = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{
		IRQHandler_UART3_Rev_Temp[IRQHandler_UART3_rev_p ++] = IRQHandler_UART3_Rev_rx;
		HAL_UART_Receive_IT(&huart3, (uint8_t*)&IRQHandler_UART3_Rev_rx, 1);
	}
}

volatile uint32_t debug_rx_event_cnt = 0;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance == USART2)
	{
		debug_rx_event_cnt ++;
		
		
//		printf("cnt:%d\r\n",debug_rx_event_cnt);
		
		uint8_t *src;
		
		if(IRQHandler_Uart2_current_rx_buf == IRQHandler_Uart2_rx_buf1)
		{
			src = IRQHandler_Uart2_rx_buf1;
		}
		else
		{
			src = IRQHandler_Uart2_rx_buf2;
		}
		
		for(uint16_t i = 0; i < Size; i++)
		{
			uint16_t next_wr = (IRQHandler_uart2_ring_wr + 1) % IRQHANDLER_UART2_RX_RING_BUF_SIZE;
			if(next_wr == IRQHandler_uart2_ring_rd) break;
			
			IRQHandler_Uart2_rx_ring_buf[IRQHandler_uart2_ring_wr] = src[i];
			IRQHandler_uart2_ring_wr = (IRQHandler_uart2_ring_wr + 1) % IRQHANDLER_UART2_RX_RING_BUF_SIZE;
		}
		
		IRQHandler_Uart2_rx_complete_flag = 1;
		
		if(g_uart2_rx_sem != 0xffff)
		{
				LOS_SemPost(g_uart2_rx_sem);
		}
		

		
		if(IRQHandler_Uart2_current_rx_buf == IRQHandler_Uart2_rx_buf1)
		{
			IRQHandler_Uart2_current_rx_buf = IRQHandler_Uart2_rx_buf2;
			HAL_UARTEx_ReceiveToIdle_DMA(&huart2, IRQHandler_Uart2_rx_buf2, IRQHANDLER_UART2_RX_BUF_SIZE);
		}
		else
		{
			IRQHandler_Uart2_current_rx_buf = IRQHandler_Uart2_rx_buf1;
			HAL_UARTEx_ReceiveToIdle_DMA(&huart2, IRQHandler_Uart2_rx_buf1, IRQHANDLER_UART2_RX_BUF_SIZE);
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}



