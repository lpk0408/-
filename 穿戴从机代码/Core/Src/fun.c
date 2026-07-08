#include "all_include.h"
#include "IRQHandler.h"
#include "task.h"
#include "nbiot.h"
#include "mpu6050.h"
#include "ina226.h"

void MCU_Peripheral_Init(void)
{
//	HAL_UART_Receive_IT(&huart2, (uint8_t*)&IRQHandler_UART2_Rev_rx, 1);
	HAL_UART_Receive_IT(&huart3, (uint8_t*)&IRQHandler_UART3_Rev_rx, 1);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2, IRQHandler_Uart2_rx_buf1, IRQHANDLER_UART2_RX_BUF_SIZE);
//	__HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
	printf("MCU_Peripheral_Init Successed!\r\n");
}

void TASK_Acitve(void)
{
	
	
	INA226_Init();
	
	mpu6050_Init();
	
	HAL_Delay(50);
	
	
	
	HAL_Delay(1000);
	
	printf("waiting for nbiot 10s ...\r\n");
	
	HAL_Delay(10000);
	
	NBIOT_CIMI();
	
	NBIOT_MQTT_Client_Create();
	
	NBIOT_MQTT_Client_Connect();

	Task_Start();
}

int fputc(int ch,FILE* f)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,1);
	
	return ch;
}
