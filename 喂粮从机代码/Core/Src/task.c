#include "all_include.h"
#include "task.h"
#include "lora.h"
#include "IRQHandler.h"
#include "hx1117.h"
#include "adc.h"
#include "sg90.h"

unsigned int MUTEX_UART2;

#define TASK_STACK_SIZE  4096

uint32_t water_val;

uint32_t water_test = 2005;

float weight_test = 200.0;

//uint16_t angle = 0;

char lora_send_weight[64];

char lora_send_water[64];

uint8_t lora_send_flag = 0;

uint8_t lora_rev_water = 0;

uint8_t lora_rev_feed = 0;

uint32_t send_timeout = 1000;

float weight_val = 0.0;

void water_get(void)
{
	HAL_ADC_Start(&hadc1);
	
	water_val = HAL_ADC_GetValue(&hadc1);
}

//void SG_SET_Angle(uint16_t angle)
//{
//	if(angle > 180) angle = 180;
//	if(angle < 0) angle = 0;
//	
//	float duty_f = 2.5 + (angle / 180.0f) * 10.0f;

//	unsigned int CCR = (unsigned int)(duty_f + 0.5);
//	
//	TIM12->CCR1 = CCR;
//}



void PrintTask_UART1(void)
{
    while (1)
		{
			
//			float weight_val = HX711_Get_Weight();
			
			weight_val = HX711_Get_Weight();
			
			water_get();
//			
//			weight_val = weight_test;
//			
//			water_val = water_test;
			
//			weight_val = HX711_Get_Weight();
			
			
			sprintf(lora_send_weight,"awe%.1lf",weight_val);
			
//			LOS_TaskDelay(10);
			
			sprintf(lora_send_water,"awt%d",water_val);
			
			printf("weight:%s\r\nwater:%s\r\n",lora_send_weight,lora_send_water);
//			
//			printf("angle:%d\r\n",angle);
//			
//			angle += 30;
			
//			if(angle > 180) angle = 0;
			
      LOS_TaskDelay(5000);
    }
}

void PrintTask_UART2(void)
{	
    while (1)
		{	
			
			if(IRQHandler_UART2_rev_final_Flag != 0)
			{
				if(strstr((const char*)IRQHandler_UART2_Rev_Temp,(const char*)"get") != NULL)
				{
					lora_send_flag = 1;
				}
				
				if(strstr((const char*)IRQHandler_UART2_Rev_Temp,(const char*)"af") != NULL)
				{
					lora_rev_feed = 1;
				}
				
				if(strstr((const char*)IRQHandler_UART2_Rev_Temp,(const char*)"aw") != NULL)
				{
					lora_rev_water = 1;
				}
				
				printf("lora_rev:%s\r\n",IRQHandler_UART2_Rev_Temp);
				
				memset(IRQHandler_UART2_Rev_Temp,0,sizeof(IRQHandler_UART2_Rev_Temp));
				
				IRQHandler_UART2_rev_final_Flag = 0;
				
				IRQHandler_UART2_rev_p = 0;
				
				LOS_TaskDelay(100);
			}
			
      LOS_TaskDelay(5);
    }
}

void PrintTask_UART3(void)
{	
    while (1)
		{		
			if(lora_send_flag)
			{
				lora_send_flag = 0;
				
				LoRa_Send_string(lora_send_weight);
				
				LOS_Msleep(send_timeout);
				
				LoRa_Send_string(lora_send_water);
				
				LOS_Msleep(send_timeout);
			}
			
			if(lora_rev_feed)
			{
				printf("feed_get\r\n");
				
				SG_SET_Angle(60);
				
				LOS_TaskDelay(500);
				
				SG_SET_Angle(0);
				
				lora_rev_feed = 0;
//				lora_send_flag = 1;
			}
				
			if(lora_rev_water)
			{
				printf("water_get\r\n");
				
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
				LOS_TaskDelay(500);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
				
				lora_rev_water = 0;
				
//				lora_send_flag = 1;
			}
			
			LOS_TaskDelay(100);
    }
}

void TASK_INIT(void)
{
	UINT32 g_printTaskId_uart_1;
	UINT32 g_printTaskId_uart_2;
	UINT32 g_printTaskId_uart_3;
	
	TSK_INIT_PARAM_S taskParam_uart_1 = {0};
	TSK_INIT_PARAM_S taskParam_uart_2 = {0};
	TSK_INIT_PARAM_S taskParam_uart_3 = {0};
	
	taskParam_uart_1.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_UART1;
	taskParam_uart_1.uwStackSize   = TASK_STACK_SIZE;
	taskParam_uart_1.pcName        = "PrintTask_UART1";
	taskParam_uart_1.usTaskPrio    = 5;                     
	LOS_TaskCreate(&g_printTaskId_uart_1, &taskParam_uart_1);
	
	taskParam_uart_2.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_UART2;
	taskParam_uart_2.uwStackSize   = TASK_STACK_SIZE;
	taskParam_uart_2.pcName        = "PrintTask_UART2";
	taskParam_uart_2.usTaskPrio    = 5;                  
	LOS_TaskCreate(&g_printTaskId_uart_2, &taskParam_uart_2);
	
	taskParam_uart_3.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_UART3;
	taskParam_uart_3.uwStackSize   = TASK_STACK_SIZE;
	taskParam_uart_3.pcName        = "PrintTask_UART3";
	taskParam_uart_3.usTaskPrio    = 5;                  
	LOS_TaskCreate(&g_printTaskId_uart_3, &taskParam_uart_3);
}

void Muetx_INIT(void)
{   
    unsigned int ret = LOS_OK;
	
    ret = LOS_MuxCreate(&MUTEX_UART2);
    if (ret != LOS_OK)
    {
        printf("Falied to create MUTEX_UART2\n");
    }
}

void Task_Start(void)
{
	LOS_KernelInit();
	Muetx_INIT();
	TASK_INIT();
	LOS_Start();
}


