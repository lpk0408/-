#include "all.h"
#include "SU_03T.h"

#define UART2_HANDLE EUART2_M1

#define MSG_QUEUE_LENGTH                                16
#define BUFFER_LEN                                      50

extern unsigned int m_su03_msg_queue;
extern bool light_state;

void su03t_send_double_msg(uint8_t index, double dat)
{
    uint8_t buf[50] = {0};
    uint8_t *buf_ptr = buf;
    uint8_t *u8_ptr = (uint8_t *)&dat;

    *buf_ptr = 0xAA;
    buf_ptr++;
    *buf_ptr = 0x55;
    buf_ptr++;
    *buf_ptr = index;
    buf_ptr++;

    for (uint8_t i = 0; i < sizeof(double); i++)
    {
        *buf_ptr = u8_ptr[i];
        buf_ptr++;
    }

    *buf_ptr = 0x55;
    buf_ptr++;
    *buf_ptr = 0xAA;

    IoTUartWrite(UART2_HANDLE, buf, sizeof(buf));
}

void su_03t_thread()
{
    IotUartAttribute attr;
    double *data_ptr = NULL;
    unsigned int ret = 0;

    IoTUartDeinit(UART2_HANDLE);
    
    attr.baudRate = 115200;
    attr.dataBits = IOT_UART_DATA_BIT_8;
    attr.pad = IOT_FLOW_CTRL_NONE;
    attr.parity = IOT_UART_PARITY_NONE;
    attr.rxBlock = IOT_UART_BLOCK_STATE_BLOCK;
    attr.stopBits = IOT_UART_STOP_BIT_1;
    attr.txBlock = IOT_UART_BLOCK_STATE_BLOCK;
    
    ret = IoTUartInit(UART2_HANDLE, &attr);
    if (ret != IOT_SUCCESS)
    {
        printf("%s, %d: IoTUartInit(%d) failed!\n", __FILE__, __LINE__, ret);
        return;
    }
    

    while(1)
    {
        uint8_t data[64] = {0};
        uint8_t rec_len = IoTUartRead(UART2_HANDLE, data, sizeof(data));

        /* 排空队列所有旧消息, 只保留最后(最新)一条 */
        {
            double *latest_ptr = NULL;
            double *tmp_ptr = NULL;
            while (LOS_QueueRead(m_su03_msg_queue, (void *)&tmp_ptr, BUFFER_LEN, LOS_NO_WAIT) == LOS_OK)
            {
                latest_ptr = tmp_ptr;
            }
            if (latest_ptr != NULL)
            {
                data_ptr = latest_ptr;
            }
            else
            {
                /* 队列为空(极端情况), 等一帧新数据 */
                LOS_QueueRead(m_su03_msg_queue, (void *)&data_ptr, BUFFER_LEN, LOS_WAIT_FOREVER);
            }
        }

        printf("uart2 read %d  bytes:\n",rec_len);
        for(int i = 0;i< rec_len;i++)
        {
            printf("%02x \r\n",data[i]);
        }
        printf("\r\n");

        if(rec_len != 0)
        {
            uint16_t command = data[0] << 8 | data[1];

            if(command == light_state_on)
            {
                printf("Light_on\n");
            }
            else if(command == light_state_off)
            {
                printf("Light_off\n");
            }
            else if(command == temperature_get)
            {
                su03t_send_double_msg(1,data_ptr[1]);
            }

        }

        LOS_Msleep(100);
    }
}


