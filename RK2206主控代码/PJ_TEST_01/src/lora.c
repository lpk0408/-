#include "UART0_M0.h"
#include "lora.h"
#include "all.h"

#define UART_BUF_MAX 256
#define AT_RETRY_MAX 10           /* AT模式切换最多重试10次 */

unsigned char lora_rev_buf[UART_BUF_MAX];

unsigned int lora_rev_len;

extern unsigned int m_mutex_UART0;

/******************************************************************
* 函数名称: Lora_Entry_AT
* 说    明: 进入AT指令模式(带超时)
* 参    数: 无
* 返 回 值: 0=成功, -1=超时失败
*******************************************************************/
int Lora_Entry_AT(void)
{
    int AT_Flag = 1;
    int retry = 0;

    const char ENTRY_AT[] = "Entry AT";

    LOS_MuxPend(m_mutex_UART0, LOS_WAIT_FOREVER);

    while (AT_Flag && retry < AT_RETRY_MAX)
    {
        lora_rev_len = 0;
        memset(lora_rev_buf, 0, sizeof(lora_rev_buf));

        UART0_M0_Print("+++\r\n");

        LOS_Msleep(1000);

        lora_rev_len = IoTUartRead(UART_HANDLE, lora_rev_buf, sizeof(lora_rev_buf));

        if(lora_rev_len != 0)
        {
            if(strstr((const char*)lora_rev_buf, ENTRY_AT) != NULL)
            {
                AT_Flag = 0;
            }
        }
        retry++;
    }

    if (AT_Flag == 0)
    {
        printf("Entry AT OK\n");
    }
    else
    {
        printf("Entry AT timeout!\n");
    }

    LOS_MuxPost(m_mutex_UART0);
    return (AT_Flag == 0) ? 0 : -1;
}

/******************************************************************
* 函数名称: Lora_Exit_AT
* 说    明: 退出AT指令模式(带超时, 兼容两种启动状态)
* 参    数: 无
* 返 回 值: 0=成功, -1=超时失败
*******************************************************************/
int Lora_Exit_AT(void)
{
    int EXIT_Flag = 1;
    int retry = 0;

    const char EXIT_AT[] = "Power on";
    const char ENTRY_AT[] = "Entry AT";

    LOS_MuxPend(m_mutex_UART0, LOS_WAIT_FOREVER);

    /* 第1步: 尝试退出AT模式 → 期望回复 "Power on" */
    while (EXIT_Flag && retry < AT_RETRY_MAX)
    {
        lora_rev_len = 0;
        memset(lora_rev_buf, 0, sizeof(lora_rev_buf));

        UART0_M0_Print("+++\r\n");

        LOS_Msleep(1000);

        lora_rev_len = IoTUartRead(UART_HANDLE, lora_rev_buf, sizeof(lora_rev_buf));

        if(lora_rev_len != 0)
        {
            if(strstr((const char*)lora_rev_buf, EXIT_AT) != NULL)
            {
                EXIT_Flag = 0;  /* 成功退出AT模式 */
            }
            else if(strstr((const char*)lora_rev_buf, ENTRY_AT) != NULL)
            {
                /* 模块之前在透明模式, +++ 使其进入了AT模式
                   需要再发一次 +++ 来退出 */
                printf("Module was in transparent mode, re-send +++\n");
                /* 再发一次 +++ 退出AT模式 */
                LOS_Msleep(1000);
                UART0_M0_Print("+++\r\n");
                LOS_Msleep(1000);
                lora_rev_len = IoTUartRead(UART_HANDLE, lora_rev_buf, sizeof(lora_rev_buf));
                if(strstr((const char*)lora_rev_buf, EXIT_AT) != NULL)
                {
                    EXIT_Flag = 0;
                }
            }
        }
        retry++;
    }

    if (EXIT_Flag == 0)
    {
        printf("EXIT AT OK\n");
    }
    else
    {
        printf("EXIT AT timeout!\n");
    }

    LOS_MuxPost(m_mutex_UART0);
    return (EXIT_Flag == 0) ? 0 : -1;
}

/******************************************************************
* 函数名称: LoRa_Send_double
* 说    明: 发送double类型数据(带互斥保护)
* 参    数: x - 要发送的double值
* 返 回 值: 无
*******************************************************************/
void LoRa_Send_double(double x)
{
    char str[32];
    sprintf(str, "%.2f", x);

    size_t str_len = strlen(str);
    /* 帧格式: [addrH][addrL][ch][payload][0x64][\r][\n] */
    size_t total_len = 3 + str_len + 3;  /* 修正: 0x64 + \r + \n = 3字节 */

    uint8_t send_data[256];

    send_data[0] = 0x11;
    send_data[1] = 0x12;
    send_data[2] = 0x01;

    for (size_t i = 0; i < str_len; i++)
    {
        send_data[3 + i] = (uint8_t)str[i];
    }

    send_data[3 + str_len] = 0x64;
    send_data[3 + str_len + 1] = 0x0D;
    send_data[3 + str_len + 2] = 0x0A;

    LOS_MuxPend(m_mutex_UART0, LOS_WAIT_FOREVER);
    UART0_M0_WriteRaw(send_data, total_len);
    LOS_MuxPost(m_mutex_UART0);
}

/******************************************************************
* 函数名称: LoRa_Send_string
* 说    明: 发送字符串命令(带互斥保护)
* 参    数: index - 地址索引 (1=地址0x1112, 其他默认0x0000)
*           str   - 要发送的字符串
* 返 回 值: 无
*******************************************************************/
void LoRa_Send_string(int index, char* str)
{
    size_t str_len = strlen(str);
    size_t total_len = 3 + str_len + 2;

    uint8_t send_data[128];

    /* 初始化地址字节, 防止使用垃圾值 */
    if(index == 1)
    {
        send_data[0] = 0x11;
        send_data[1] = 0x12;
    }
    else
    {
        send_data[0] = 0x00;
        send_data[1] = 0x00;
    }
    send_data[2] = 0x01;

    for (size_t i = 0; i < str_len; i++)
    {
        send_data[3 + i] = (uint8_t)str[i];
    }

    /* 帧尾: \r\n */
    send_data[3 + str_len] = 0x0D;
    send_data[3 + str_len + 1] = 0x0A;

    LOS_MuxPend(m_mutex_UART0, LOS_WAIT_FOREVER);
    UART0_M0_WriteRaw(send_data, total_len);
    LOS_MuxPost(m_mutex_UART0);

    /* 给LoRa模块留出发送时间 (9600bps 约1ms/byte, 留足余量) */
    LOS_Msleep(50);
}
