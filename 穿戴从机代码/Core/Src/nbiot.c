#include "all_include.h"
#include "IRQHandler.h"
#include "nbiot.h"

#define MQTT_IP						"101.35.238.82"
#define MQTT_PORT					"1883"

//#define MQTT_IP						"29.tcp.cpolar.top"
//#define MQTT_PORT					"13949"

#define MQTT_CLIENT_TIME	"5000"
#define MQTT_CLIENT_SIZE	"1000"

#define MQTT_TOPIC				"test"

void StringToHexStr(char* src, char* dest, int max_len)
{
	const char hex_table[] = "0123456789ABCDEF";
	int i = 0;
	while (*src && (i + 2) < max_len)
	{
			dest[i++] = hex_table[(*src >> 4) & 0x0F];
			dest[i++] = hex_table[*src & 0x0F];
			src++;
	}
	dest[i] = '\0';
}

void NBIOT_CIMI(void)
{
	memset(IRQHandler_UART3_Rev_Temp,0,sizeof(IRQHandler_UART3_Rev_Temp));
				
	IRQHandler_UART3_rev_p = 0;
	
	printf("AT+CIMI Check:\r\n");
	
	HAL_UART_Transmit(&huart3,(uint8_t*)"AT+CIMI\r\n",9	,0xffff);
	
	HAL_Delay(500);
	
	if(IRQHandler_UART3_rev_p)
	{
		if(strstr((const char*)IRQHandler_UART3_Rev_Temp,(const char*)"+CIMI:460041472616790") != NULL)
		{
			printf("NBIOT CIMI Good!\r\n");
		}
	}
}

void NBIOT_MQTT_Client_Create(void)
{
	memset(IRQHandler_UART3_Rev_Temp,0,sizeof(IRQHandler_UART3_Rev_Temp));
				
	IRQHandler_UART3_rev_p = 0;
	
	printf("NBIOT_MQTT_Client_Create:\r\n");
	
	char AT_TEMP[128] = {0};
	
	//AT+MQNEW=29.tcp.cpolar.top,13949,5000,1000\r\n
	//AT+MQNEW=101.35.238.82,1883,5000,1000\r\n
	sprintf(AT_TEMP,"AT+MQNEW=%s,%s,%s,%s\r\n",MQTT_IP,MQTT_PORT,MQTT_CLIENT_TIME,MQTT_CLIENT_SIZE);
	
	HAL_UART_Transmit(&huart3,(uint8_t*)AT_TEMP,strlen(AT_TEMP),0xffff);
	
	HAL_Delay(4000);
	
	if(IRQHandler_UART3_rev_p)
	{
		printf("MQNEW resp: %s\r\n", IRQHandler_UART3_Rev_Temp);
		if(strstr((const char*)IRQHandler_UART3_Rev_Temp,(const char*)"+MQNEW:0") != NULL)
		{
			printf("NBIOT_MQTT_Client_Create Successed!\r\n");
		}
	}
	else
	{
		printf("MQNEW: no response!\r\n");
	}
}

void NBIOT_MQTT_Client_Connect(void)
{
	memset(IRQHandler_UART3_Rev_Temp,0,sizeof(IRQHandler_UART3_Rev_Temp));
				
	IRQHandler_UART3_rev_p = 0;
	
	printf("NBIOT_MQTT_Client_Connect :\r\n");
	
	//AT+MQCON=0,4,"qs-100",60,1,0\r\n
	
	char AT_TEMP[] = "AT+MQCON=0,4,\"qs-100\",60,1,0\r\n";
	
	HAL_UART_Transmit(&huart3,(uint8_t*)AT_TEMP,strlen(AT_TEMP),0xffff);
	
//	LOS_TaskDelay(100);
	
	HAL_Delay(3000);
	
	if(IRQHandler_UART3_rev_p)
	{
		if(strstr((const char*)IRQHandler_UART3_Rev_Temp,(const char*)"OK") != NULL)
		{
			printf("NBIOT_MQTT_Client_Connect Successed!\r\n");
		}
	}
}

void NBIOT_MQTT_Send_Message(char* tpoic,char* message)
{
	memset(IRQHandler_UART3_Rev_Temp,0,sizeof(IRQHandler_UART3_Rev_Temp));
	
	IRQHandler_UART3_rev_p = 0;
	
//	printf("NBIOT_MQTT_Send_Message :\r\n");
	
	char AT_TEMP[128] = {0};
	
	char hex_message[256] = {0};
	
	StringToHexStr(message, hex_message, sizeof(hex_message));
	
	int Message_len = strlen(hex_message) / 2;
	
	sprintf(AT_TEMP,"AT+MQPUB=0,\"%s\",1,0,0,%d,%s\r\n",tpoic,Message_len,hex_message);
	
	int len = strlen(AT_TEMP);
	
	HAL_UART_Transmit(&huart3,(uint8_t*)AT_TEMP,len,100);
	
	LOS_Msleep(100);
	
	if(IRQHandler_UART3_rev_p)
	{
		if(strstr((const char*)IRQHandler_UART3_Rev_Temp,(const char*)"OK") != NULL)
		{
			printf("Message:%s Send Successed!\r\n",message);
		}
	}
}


