#include "all_include.h"
#include "task.h"
#include "IRQHandler.h"
#include "nbiot.h"
#include "gps.h"
#include "mpu6050.h"
#include "rtc.h"
#include "los_pm.h"
#include "ina226.h"

unsigned int MUTEX_UART2;

unsigned int MUTEX_NBIOT;

uint32_t send_timeout = 1000;

#define MAX_COUNT 0          
UINT32 g_uart2_rx_sem = 0xffff;

#define TASK_STACK_SIZE  1024

uint8_t UART2_TEST_Flag = 0;

//char *lon_buf = NULL;

//char *lat_buf = NULL;

char lon_buf[32] = {0};
char lat_buf[32] = {0};


uint8_t GPS_Location_Flag;

extern uint8_t GPS_Status;
extern uint8_t GPS_Antenna_OK;

extern unsigned long  step_count;

char step_temp[32] = {0};

static unsigned long pre_step;

static unsigned long cur_step;

char lat[20] = {0}, ns[2] = {0};
char lon[20] = {0}, ew[2] = {0};

int low_flag = 0;

float soc_val;

unsigned long task_step;

uint8_t step_send_flag;

void entry_low_pwr(void)
{
	__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
	__HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
	
	HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);

	SystemClock_Config(); 
}

static void process_nmea_line(char *line)
{
	// 1. 检测天线状态（GPS模块私有语句或文本消息）
	if (strstr(line, "ANTENNA") != NULL || strstr(line, "antenna") != NULL)
	{
		if (strstr(line, "OK") != NULL || strstr(line, "ok") != NULL ||
		    strstr(line, "OPEN") != NULL || strstr(line, "open") != NULL ||
		    strstr(line, "Normal") != NULL || strstr(line, "normal") != NULL)
		{
			if (GPS_Antenna_OK == 0)
			{
				GPS_Antenna_OK = 1;
				printf("【GPS 状态】天线状态: 正常\r\n");
			}
		}
		else if (strstr(line, "SHORT") != NULL || strstr(line, "short") != NULL ||
		         strstr(line, "FAULT") != NULL || strstr(line, "fault") != NULL)
		{
			GPS_Antenna_OK = 0;
			printf("【GPS 状态】天线状态: 异常! (%s)\r\n", line);
		}
	}

	// 2. 检测任何NMEA语句，标记GPS模块已上电开始通信
	if ((strncmp(line, "$GN", 3) == 0 || strncmp(line, "$GP", 3) == 0 ||
	     strncmp(line, "$GL", 3) == 0 || strncmp(line, "$BD", 3) == 0 ||
	     strncmp(line, "$GA", 3) == 0 || strncmp(line, "$PQ", 3) == 0 ||
	     strncmp(line, "$PM", 3) == 0 || strncmp(line, "$PS", 3) == 0) &&
	    GPS_Status == GPS_OFFLINE)
	{
		GPS_Status = GPS_STARTING;
		printf("【GPS 状态】检测到GPS模块已上电通信，冷启动中，正在搜星...\r\n");
	}

	// 3. 解析$GNGGA获取定位信息
	if (strncmp(line, "$GNGGA", 6) == 0)
	{
		Get_NMEA_Field(line, 2, lat);
		Get_NMEA_Field(line, 3, ns);
		Get_NMEA_Field(line, 4, lon);
		Get_NMEA_Field(line, 5, ew);

		if (lat[0] != '\0' && lon[0] != '\0')
		{
			strncpy(lat_buf, lat, sizeof(lat_buf) - 1);
			strncpy(lon_buf, lon, sizeof(lon_buf) - 1);
			GPS_Location_Flag = 1;

			if (GPS_Status != GPS_FIXED)
			{
				GPS_Status = GPS_FIXED;
				printf("【GPS 状态】冷启动完成，已成功定位!\r\n");
			}
			printf("【GPS 定位信息】纬度: %s %s, 经度: %s %s\r\n", lat, ns, lon, ew);
		}
		else
		{
			GPS_Location_Flag = 0;
			// 收到GNGGA但无有效定位，标记GPS已启动
			if (GPS_Status < GPS_STARTING)
			{
				GPS_Status = GPS_STARTING;
			}
			// 减少重复打印: 只在首次提示
			static uint8_t no_fix_printed = 0;
			if (!no_fix_printed)
			{
				printf("【GPS 状态】收到GNGGA数据但暂无定位，等待搜星完成...\r\n");
				no_fix_printed = 1;
			}
		}
	}
}

void PrintTask_UART1(void)
{
	uint8_t ch;
	char line_buf[256];
	int line_idx = 0;

	while (1)
	{
		if (low_flag) { LOS_Msleep(5000); continue; }

		UINT32 ret = LOS_SemPend(g_uart2_rx_sem, LOS_WAIT_FOREVER);
		if (ret != LOS_OK) continue;

		for (;;)
		{
			if (IRQHandler_uart2_ring_wr == IRQHandler_uart2_ring_rd)
				break;

			ch = IRQHandler_Uart2_rx_ring_buf[IRQHandler_uart2_ring_rd];
			IRQHandler_uart2_ring_rd = (IRQHandler_uart2_ring_rd + 1) % IRQHANDLER_UART2_RX_RING_BUF_SIZE;

			if (ch == '\n' || ch == '$')
			{
				if (ch == '$' && line_idx == 0)
				{
					line_buf[line_idx++] = ch;
					continue;
				}

				line_buf[line_idx] = '\0';
				process_nmea_line(line_buf);
				line_idx = 0;

				if (ch == '$')
					line_buf[line_idx++] = ch;
			}
			else if (ch != '\r' && line_idx < (int)sizeof(line_buf) - 1)
			{
				line_buf[line_idx++] = ch;
			}
		}
		LOS_Msleep(100);
	}
}


//void PrintTask_UART2(void)
//{	
//	while (1)
//	{
//		printf("task2\r\n");
//	
//		if(GPS_Location_Flag)
//		{
//		NBIOT_MQTT_Send_Message("f407/GPS_lon",lon_buf);
//		
//		LOS_Msleep(2000);
//		
//		NBIOT_MQTT_Send_Message("f407/GPS_lat",lat_buf);
//		}
//		else
//		{
//		NBIOT_MQTT_Send_Message("f407/GPS_lon","3517.62283");
//		
//		LOS_Msleep(2000);
//		
//		NBIOT_MQTT_Send_Message("f407/GPS_lat","11356.19352");
//		}
//		
//		LOS_Msleep(2000);
//		
//		sprintf(step_temp,"%lu",task_step);
//			
//		NBIOT_MQTT_Send_Message("f407/step",step_temp);
//		
//		LOS_Msleep(2000);
//		
//		char soc_temp[24] = {0};
//		
//		sprintf(soc_temp,"%.1lf",soc_val);
//		
//		NBIOT_MQTT_Send_Message("f407/bat",soc_temp);
//		
//		LOS_Msleep(2000);
//	}
//}

void PrintTask_UART2(void)
{
	while (1)
	{
		printf("task2 GPS状态:%s\r\n", GPS_GetStatusString());

		if(GPS_Location_Flag)
		{
		NBIOT_MQTT_Send_Message("f407/GPS_lon",lon_buf);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
		LOS_Msleep(send_timeout);

		NBIOT_MQTT_Send_Message("f407/GPS_lat",lat_buf);
		}
		else
		{
		NBIOT_MQTT_Send_Message("f407/GPS_lon","11356.19352");

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
		LOS_Msleep(send_timeout);

		NBIOT_MQTT_Send_Message("f407/GPS_lat","3517.62283");
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
		LOS_Msleep(send_timeout);
		}
		
		if(step_send_flag)
		{
			step_send_flag = 0;
			sprintf(step_temp,"%lu",task_step);
			NBIOT_MQTT_Send_Message("f407/step",step_temp);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
			LOS_Msleep(send_timeout);
		}
		
		char soc_temp[24] = {0};
		
		sprintf(soc_temp,"%.1lf",soc_val);
		
		NBIOT_MQTT_Send_Message("f407/bat",soc_temp);
		
		LOS_Msleep(send_timeout);
	}
}

void PrintTask_UART3(void)
{	
	while(1)
	{
			soc_val = get_bat_soc();
			
			LOS_Msleep(100);
	}
}

void PrintTask_STEP(void)
{	
	while(1)
	{
			mpu6050_step_detect_and_print();
		
			task_step = mpu6050_get_step_count();
			
			if(pre_step != task_step)
			{
				step_send_flag = 1;
				pre_step = task_step;
			}
		
			LOS_Msleep(25);
	}
}

//void PrintTask_TASK4(void)
//{	
//	while(1)
//	{
//		printf("soc:%.1f\r\n",soc_val);
//		
//		printf("step:%lu\r\n",task_step);
//		
//		LOS_Msleep(5000);
//	}
//}

void PrintTask_TASK4(void)
{	
	while(1)
	{
		printf("soc:%.1f\r\n",soc_val);
		
		printf("step:%lu\r\n",task_step);
		
		printf("vol:%d\r\n",INA226_GetBusVoltage());
		
		LOS_Msleep(5000);
	}
}

void TASK_INIT(void)
{
	UINT32 g_printTaskId_uart_1;
	UINT32 g_printTaskId_uart_2;
	UINT32 g_printTaskId_uart_3;
	UINT32 g_printTaskId_uart_4;
	UINT32 g_printTaskId_uart_5;
	
	TSK_INIT_PARAM_S taskParam_uart_1 = {0};
	TSK_INIT_PARAM_S taskParam_uart_2 = {0};
	TSK_INIT_PARAM_S taskParam_uart_3 = {0};
	TSK_INIT_PARAM_S taskParam_uart_4 = {0};
	TSK_INIT_PARAM_S taskParam_uart_5 = {0};
	
	unsigned int ret = LOS_OK;
	
	taskParam_uart_1.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_UART1;
	taskParam_uart_1.uwStackSize   = 8192 + 1024;
	taskParam_uart_1.pcName        = "PrintTask_UART1";
	taskParam_uart_1.usTaskPrio    = 6;                     
	ret = LOS_TaskCreate(&g_printTaskId_uart_1, &taskParam_uart_1);
	
	taskParam_uart_2.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_UART2;
	taskParam_uart_2.uwStackSize   = 8192 + 1024;
	taskParam_uart_2.pcName        = "PrintTask_UART2";
	taskParam_uart_2.usTaskPrio    = 6;                  
	ret = LOS_TaskCreate(&g_printTaskId_uart_2, &taskParam_uart_2);
	
	taskParam_uart_3.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_UART3;
	taskParam_uart_3.uwStackSize   = 2048;   /* 原1024过小，增加余量防止中断嵌套溢出 */
	taskParam_uart_3.pcName        = "PrintTask_UART3";
	taskParam_uart_3.usTaskPrio    = 5;                  
	ret = LOS_TaskCreate(&g_printTaskId_uart_3, &taskParam_uart_3);
	
	taskParam_uart_4.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_TASK4;
	taskParam_uart_4.uwStackSize   = 4096;   /* 原1024严重不足！printf+浮点格式化需要2KB+栈空间 */
	taskParam_uart_4.pcName        = "PrintTask_UART4";
	taskParam_uart_4.usTaskPrio    = 5;                  
	ret = LOS_TaskCreate(&g_printTaskId_uart_4, &taskParam_uart_4);
	
	taskParam_uart_5.pfnTaskEntry = (TSK_ENTRY_FUNC)PrintTask_STEP;
	taskParam_uart_5.uwStackSize   = 4096 + 2048;
	taskParam_uart_5.pcName        = "PrintTask_UART5";
	taskParam_uart_5.usTaskPrio    = 5;                  
	ret = LOS_TaskCreate(&g_printTaskId_uart_5, &taskParam_uart_5);
}

void Muetx_INIT(void)
{   
    unsigned int ret = LOS_OK;
	
    ret = LOS_MuxCreate(&MUTEX_UART2);
    if (ret != LOS_OK)
    {
        printf("Falied to create MUTEX_UART2\n");
    }
		
		ret = LOS_MuxCreate(&MUTEX_NBIOT);
    if (ret != LOS_OK)
    {
        printf("Falied to create MUTEX_NBIOT\n");
    }
}

void Sem_INIT(void)
{
	unsigned int ret = LOS_OK;

	ret = LOS_SemCreate(MAX_COUNT, &g_uart2_rx_sem);
	
	if (ret != LOS_OK)
	{
			printf("Falied to create Semaphore\n");
			return;
	}
}

void Task_Start(void)
{
	LOS_KernelInit();
	
	Sem_INIT();
	
	Muetx_INIT();
	
	TASK_INIT();
	
	LOS_PmModeSet(LOS_SYS_NORMAL_SLEEP);
	
	LOS_Start();
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
	
}
