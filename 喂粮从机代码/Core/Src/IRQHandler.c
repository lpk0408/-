#include "IRQHandler.h"

uint8_t IRQHandler_UART2_Rev_Temp[HEX_SEND_BUF_SIZE];

char IRQHandler_UART2_Send_buf[CHAR_SEND_BUF_SIZE];

uint8_t IRQHandler_UART2_Rev_rx;

volatile uint32_t IRQHandler_UART2_rev_p;

volatile uint8_t IRQHandler_UART2_rev_final_Flag;

uint8_t IRQHandler_UART3_Rev_Temp[HEX_SEND_BUF_SIZE];

uint8_t IRQHandler_UART3_Rev_rx;

volatile uint32_t IRQHandler_UART3_rev_p;

volatile uint8_t IRQHandler_UART3_rev_final_Flag;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)
	{
		HAL_UART_Transmit(&huart1,&IRQHandler_UART2_Rev_rx,1,0xffff);
		
		if(IRQHandler_UART2_Rev_rx == 'd') IRQHandler_UART2_rev_final_Flag = 1;
		
		if(!IRQHandler_UART2_rev_final_Flag) IRQHandler_UART2_Rev_Temp[IRQHandler_UART2_rev_p ++] = IRQHandler_UART2_Rev_rx;
		
		
		
		HAL_UART_Receive_IT(&huart2, (uint8_t*)&IRQHandler_UART2_Rev_rx, 1);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}

