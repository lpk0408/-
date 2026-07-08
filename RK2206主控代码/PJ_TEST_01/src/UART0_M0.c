#include "all.h"
#include "UART0_M0.h"

//EUART0_M0     RX GPIO0_PB6 TX GPIO0_PB7 

// #define UART_HANDLE EUART0_M0

#define UART_BUF_MAX 256

unsigned char uart_rev_buf[UART_BUF_MAX];

unsigned int rev_len;

extern unsigned int m_mutex_UART0;

char a_weight[64];

char a_water[64];

volatile uint8_t a_flag = 0;

uint8_t a_finish_flag = 0;

void UART0_MO_Init(void)
{
    IotUartAttribute attr;

    unsigned int ret = 0;

    IoTUartDeinit(UART_HANDLE);

    attr.baudRate = 9600;
    attr.dataBits = IOT_UART_DATA_BIT_8;
    attr.pad = IOT_FLOW_CTRL_NONE;
    attr.parity = IOT_UART_PARITY_NONE;
    attr.rxBlock = IOT_UART_BLOCK_STATE_BLOCK;
    attr.stopBits = IOT_UART_STOP_BIT_1;
    attr.txBlock = IOT_UART_BLOCK_STATE_BLOCK;
    
    ret = IoTUartInit(UART_HANDLE, &attr);

    if (ret != IOT_SUCCESS)
    {
        printf("%s, %d: IoTUartInit(%d) failed!\n", __FILE__, __LINE__, ret);
        return;
    }
}

void UART0_M0_Print(const char *buf)
{
    IoTUartWrite(UART_HANDLE, (uint8_t*)buf, strlen(buf));
}

void UART0_M0_WriteRaw(const uint8_t *buf, size_t len)
{
    IoTUartWrite(UART_HANDLE, buf, len);
}

void UART0_TEST(void)
{
    IoTUartWrite(UART_HANDLE, (uint8_t*)"hello\n", 7);
}


static char rx_frame_buf[128] = {0};
static uint32_t rx_index = 0;

void UART0_rev_Process(void)
{
    char tmp_buf[32] = {0}; 
    int temp_len = 0;

    while(1)
    {
        temp_len = IoTUartRead(UART_HANDLE, tmp_buf, sizeof(tmp_buf) - 1);

        if(temp_len > 0)
        {
            for(int i = 0; i < temp_len; i++)
            {
                if(rx_index < (sizeof(rx_frame_buf) - 1))
                {
                    rx_frame_buf[rx_index++] = tmp_buf[i];

                    // 依然是以 '\n' 作为一帧的结束标志
                    if(tmp_buf[i] == '\n')
                    {
                        rx_frame_buf[rx_index] = '\0'; 

                        // 1. 处理重量数据 (匹配 awe 后面直接接字符串)
                        const char* awe_ptr = strstr(rx_frame_buf, "awe");
                        if(awe_ptr != NULL)
                        {
                            memset(a_weight, 0, sizeof(a_weight));
                            // 🛠️ 极致精简：直接用 %s 匹配！
                            // sscanf 读到后面的 \r 或 \n 时会自动停止，a_weight 里面就是干净的 "200.00"
                            if (sscanf(awe_ptr, "awe%31s", a_weight) == 1)
                            {
                                // printf("get_a_weight: %s\r\n", a_weight);

                                a_flag ++;
                            }
                        }

                        // 2. 处理水质/水位数据
                        const char* awt_ptr = strstr(rx_frame_buf, "awt");
                        if(awt_ptr != NULL)
                        {
                            memset(a_water, 0, sizeof(a_water));
                            // 🛠️ 极致精简：同理，%s 遇到换行自动切断
                            if (sscanf(awt_ptr, "awt%31s", a_water) == 1)
                            {
                                // printf("get_a_water: %s\r\n", a_water);
                                a_flag ++;
                            }

                            
                        }

                        // 清空主缓冲区，准备接收下一帧
                        memset(rx_frame_buf, 0, sizeof(rx_frame_buf));
                        rx_index = 0;
                    }
                }
                else
                {
                    memset(rx_frame_buf, 0, sizeof(rx_frame_buf));
                    rx_index = 0;
                }
            }
            memset(tmp_buf, 0, sizeof(tmp_buf));
        }
        LOS_Msleep(10);
    }
}

void UART0_GET()
{
    
}
