#include "lora.h"
#include "all_include.h"
#include "IRQHandler.h"

void LoRa_Exit_AT(void)
{
	int AT_Flag = 1;

	int time_out = 5;
	
	const char EXIT_AT[] = "Power on";
	
	time_out = 5;
	
	while(AT_Flag && time_out > 0)
	{	
		time_out --;
		
		memset(IRQHandler_UART2_Rev_Temp,0,sizeof(IRQHandler_UART2_Rev_Temp));
				
		IRQHandler_UART2_rev_p = 0;
		
		HAL_UART_Transmit(&huart2,(uint8_t*)"+++\r\n",5,0xffff);

		HAL_Delay(500);
		
		if(IRQHandler_UART2_rev_p)
		{
			if(strstr((const char*)IRQHandler_UART2_Rev_Temp,EXIT_AT) != NULL) AT_Flag = 0;
		}
	}
	
	if(time_out)	printf("Power on\r\n");
	else printf("LoRa timeout!\r\n");
	
	printf("timeout:%d\r\n",time_out);

}

void LoRa_Send_double(double x)
{
		char str[32];   
    sprintf(str, "%.2f", x); 

    size_t str_len = strlen(str);
    size_t total_len = 3 + str_len + 2; 

    uint8_t send_data[256];
		
		//接收方地址	13,14
		//接收方信道	01
    send_data[0] = 0x13;
    send_data[1] = 0x14;
    send_data[2] = 0x01;
		
    for (size_t i = 0; i < str_len; i++)
		{
        send_data[3 + i] = (uint8_t)str[i];
    }
		
		send_data[3 + str_len] = 0x64;
		
		//结束末尾	+	\r\n
    send_data[3 + str_len + 1] = 0x0D;   		// '\r'
    send_data[3 + str_len + 2] = 0x0A; 	// '\n'

    HAL_UART_Transmit(&huart2, send_data, total_len, 20);
}

void LoRa_Send_string(char* str)
{  
    size_t str_len = strlen(str);
    size_t total_len = 3 + str_len + 2; 

    uint8_t send_data[128];
		
		//接收方地址	13,14
		//接收方信道	01
    send_data[0] = 0x13;
    send_data[1] = 0x14;
    send_data[2] = 0x01;
		
    for (size_t i = 0; i < str_len; i++)
		{
        send_data[3 + i] = (uint8_t)str[i];
    }
		
//		send_data[3 + str_len] = 0x64;
		
		//结束末尾	+	\r\n
    send_data[3 + str_len] = 0x0D;   		// '\r'
    send_data[3 + str_len + 1] = 0x0A; 	// '\n'
		
		HAL_UART_Transmit(&huart1, send_data, total_len, 50);
		
		
		
    HAL_UART_Transmit(&huart2, send_data, total_len, 50);
}





