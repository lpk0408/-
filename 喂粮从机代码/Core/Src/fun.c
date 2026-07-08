#include "all_include.h"
#include "IRQHandler.h"
#include "lora.h"
#include "task.h"
#include "sg90.h"

void MCU_Peripheral_Init(void)
{
	HAL_UART_Receive_IT(&huart2, (uint8_t*)&IRQHandler_UART2_Rev_rx, 1);
	HAL_UART_Receive_IT(&huart3, (uint8_t*)&IRQHandler_UART3_Rev_rx, 1);
	printf("MCU_Peripheral_Init Successed!\r\n");
}

void TASK_Acitve(void)
{
	
	SG_SET_Angle(0);
	
	LoRa_Exit_AT();
	
	LoRa_Send_string("lora 11 12 run!");
	
	Task_Start();
}

int fputc(int ch,FILE* f)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,0xffff);
	
	return ch;
}




