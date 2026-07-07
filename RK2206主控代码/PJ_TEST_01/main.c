#include "all.h"
#include "BEEP.h"
#include "KEY.h"
#include "SU_03T.h"
#include "UART0_M0.h"
#include "lora.h"
#include "wifi.h"
#include "pwm_sg.h"
#include "gpio_con.h"
#include "lcd.h"
#include <stdlib.h>

/*环境数据消息队列及其参数*/
#define MSG_QUEUE_LENGTH 16
#define BUFFER_LEN 50
#define LORA_CMD_GUARD_MS 500    /* LoRa相邻两次发送的最小间隔 */
static unsigned int m_msg_queue;
static unsigned int mqtt_msg_queue;
double read_data[5] = {0};

/* 人体感应传感器GPIO */
#define GPIO_BODY_INDUCTION      GPIO0_PA3

/*D8 LED灯gpio*/
#define GPIO_LED               GPIO0_PA5

/* 记录中断触发次数 */
static unsigned int m_gpio_interrupt_count = 0;
uint8_t light_flag= 0;
uint8_t body_tick = 0;

/*KEY变量*/
float key_voltage;
uint8_t KEY_VAL,KEY_DOWN,KEY_UP,KEY_OLD;

/*SU_03T*/
unsigned int m_su03_msg_queue;
bool light_state;

/*串口0互斥锁*/
unsigned int m_mutex_UART0;

/*wifi互斥锁*/
unsigned int m_mutex_WIFI;

extern volatile int g_wifi_connected;

extern volatile int g_mqtt_connected;

/*舵机占空比*/
extern unsigned int sg_duty;

unsigned int SG_Angle = 0;


/*加湿 风扇 io*/
char temp_buf[24] = {0};

/*屏幕*/
char LCD_BUF[50] = {0};

double lcd_temp,lcd_humi,lcd_dat,lcd_gas;

uint8_t LCD_switch = 0;

double lcd_m1_weight = 0.0;
int    lcd_m1_water  = 0;

/*lora*/
// extern uint8_t lora_get_flag;

/*beep*/
uint8_t beep_flag = 0;

uint8_t beep_dis_flag = 0;

void Debug_Test_thread(void *arg)
{
    double *data_ptr = NULL;

    while (1)
    {
        // printf("=================Running==================\n");
        LOS_QueueRead(m_msg_queue, (void *)&data_ptr, BUFFER_LEN, LOS_WAIT_FOREVER);
        
        // printf("hb1750:%.2lf \nsht30_temp:%.2lf\nsht30_humi:%.2lf\n",data_ptr[0],data_ptr[1],data_ptr[2]);
        // printf("gas:%.2lf\n",data_ptr[3]);

        // printf("debug_temp:%.2lf\r\n",data_ptr[1]);

        // printf("body count = %d\n", m_gpio_interrupt_count);

        lcd_temp = data_ptr[1];
        lcd_humi = data_ptr[2];
        lcd_dat = data_ptr[0];
        lcd_gas = data_ptr[3];

        if(lcd_temp > 31)
        {
           beep_flag ++;
        }
        else
        {
            if(beep_flag > 0) beep_flag --;
        }

        if(lcd_humi > 100)
        {
           beep_flag ++;
        }
        else
        {
            if(beep_flag > 0) beep_flag --;
        }

        if(lcd_gas > 3000)
        {
           beep_flag ++;
        }
        else
        {
            if(beep_flag > 0) beep_flag --;
        }

        if(beep_dis_flag == 0)
        {
        if(beep_flag > 0) BEEP_ENABLE();
        else BEEP_DISABLE();
        }
        else
        {
            BEEP_DISABLE();
        }
        LOS_Msleep(2000);
    }  
}

void mqtt_send_process(void)
{
    double *data_ptr = NULL;
    
    while (1)
    {   

        // printf("Waiting for mqtt connection...\n");
        while (!g_mqtt_connected) {
            osDelay(100);   // 每100ms检查一次
        }

        // // printf("=================Running==================\n");
        LOS_QueueRead(mqtt_msg_queue, (void *)&data_ptr, BUFFER_LEN, LOS_WAIT_FOREVER);
        
        // printf("mqtt_temp:%.2lf\r\n",data_ptr[1]);

        // char temp_buf[24] = {0};

        sprintf(temp_buf,"%.2lf",data_ptr[1]);
        // lcd_temp = data_ptr[1];
        mqtt_publish_message((const char*)"rk2206/temp",(const char*)temp_buf);
        
        LOS_Msleep(1000);

        sprintf(temp_buf,"%.2lf",data_ptr[2]);
        // lcd_humi = data_ptr[2];
        mqtt_publish_message((const char*)"rk2206/humi",(const char*)temp_buf);

        LOS_Msleep(1000);

        sprintf(temp_buf,"%.2lf",data_ptr[0]);
        // lcd_dat = data_ptr[0];
        mqtt_publish_message((const char*)"rk2206/dat",(const char*)temp_buf);

        LOS_Msleep(1000);

        sprintf(temp_buf,"%.2lf",data_ptr[3]);
        // lcd_dat = data_ptr[0];
        mqtt_publish_message((const char*)"rk2206/gas",(const char*)temp_buf);

        LOS_Msleep(1000);


        // if(a_flag >= 2)
        // {
        //     printf("a_water:%s\r\n",a_water);
        //     printf("a_weight:%s\r\n",a_weight);

        //     mqtt_publish_message((const char*)"rk2206/grain_1",(const char*)a_weight);
        //     LOS_Msleep(1000);
        //     mqtt_publish_message((const char*)"rk2206/water_1",(const char*)a_water);
        //     // a_flag = 0;
        // }
        // else
        // {
        //     LoRa_Send_string(1,"getd");
        //     LOS_Msleep(3000);
        // }


        // LOS_Msleep(1000);
    }  
}

void device_read_thread(void *arg)
{
    while (1)
    {
        bh1750_read_data(&read_data[0]);
        sht30_read_data(&read_data[1],&read_data[2]);
        // read_data[3] = get_mq2_ppm();

        unsigned int raw_gas = (unsigned int)get_mq2_ppm(); 
        read_data[3] = (double)(*(float*)&raw_gas);

        // printf("DEBUG SEND: %.2f, %.2f, %.2f\n", read_data[0], read_data[1], read_data[3]);
        LOS_QueueWrite(m_msg_queue, (void *)&read_data, sizeof(read_data), LOS_WAIT_FOREVER);
        LOS_QueueWrite(mqtt_msg_queue, (void *)&read_data, sizeof(read_data), LOS_WAIT_FOREVER);
        LOS_QueueWrite(m_su03_msg_queue, (void *)&read_data, sizeof(read_data), LOS_NO_WAIT);
        LOS_Msleep(1000);
    }
}

void gpio_isr_func(void *args)
{
    printf("check Body!\n");
    // 发生人体感应,点亮LED
    IoTGpioSetOutputVal(GPIO_LED, IOT_GPIO_VALUE1);
    light_flag= 1;
    body_tick = 0;
    m_gpio_interrupt_count ++;
}

void body_process()
{
    unsigned int ret;
   
    //初始化IO口
    IoTGpioInit(GPIO_LED);
    //设置GPIO为输出方向
    IoTGpioSetDir(GPIO_LED, IOT_GPIO_DIR_OUT);

    /* 初始化引脚为GPIO */
    IoTGpioInit(GPIO_BODY_INDUCTION);
    /* 引脚配置为输入 */
    IoTGpioSetDir(GPIO_BODY_INDUCTION, IOT_GPIO_DIR_IN);
    /* 设置中断触发方式为上升沿和中断处理函数 */
    ret = IoTGpioRegisterIsrFunc(GPIO_BODY_INDUCTION, 
        IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_RISE_LEVEL_HIGH, gpio_isr_func, NULL);
    if (ret != IOT_SUCCESS)
    {
        printf("IoTGpioRegisterIsrFunc failed(%d)\n", ret);
        return;
    }

    /* 初始化中断触发次数 */
    // m_gpio_interrupt_count = 0;
    /* 关闭中断屏蔽 */
    IoTGpioSetIsrMask(GPIO_BODY_INDUCTION, FALSE);
   
    while (1)
    {
        // printf("***************GPIO Interrupt Example*************\n");
        // printf("gpio interrupt count = %d\n", m_gpio_interrupt_count);
        // printf("\n");
        if(light_flag==1)
        {

            body_tick++;

            if(body_tick >=5)
            {
                //10s后关闭起夜灯,所有标志回复初始状态
                IoTGpioSetOutputVal(GPIO_LED, IOT_GPIO_VALUE0);
                body_tick = 0;
                light_flag= 0;
            }
            
        }
        
        /* 睡眠1秒 */
        LOS_Msleep(1000);
    }
}

void KEY_process()
{
    while (1)
    {
        key_voltage     = adc_get_voltage();

        if(key_voltage >3.2)
        {
            KEY_VAL = 0;
        }else if (key_voltage > 1.50)
        {
            // printf("LEFT\n");
            KEY_VAL = 1;
        }else if (key_voltage > 1.0)
        {
            // printf("DOWN\n");
            KEY_VAL = 2;
        }else if (key_voltage > 0.5)
        {
            // printf("RIGHT\n");
            KEY_VAL = 3;
        }else{
            // printf("UP\n");
            KEY_VAL = 4;
        }

        KEY_DOWN    = KEY_VAL & (KEY_VAL ^ KEY_OLD);
        KEY_UP      = ~ KEY_VAL & (KEY_VAL ^ KEY_OLD);
        KEY_OLD     = KEY_VAL;

        switch (KEY_DOWN)
        {
        case 1:
        {   
            printf("LEFT\n");
            // BEEP_ENABLE();

            beep_dis_flag ^= 1;

            break;
        }
        case 2:
        {
            printf("DOWN\n");

            if(LCD_switch == 1) LoRa_Send_string(1,"awd");

            // SG_SET_Angle(0);

            break;
        }
        case 3:
        {
            printf("RIGHT\n");

            LCD_switch ++;
            if(LCD_switch >= 2) LCD_switch = 0;



            break;
        }
        case 4:
        {   
            // Lora_Exit_AT();
            printf("UP\n");

            // SG_Angle += 30;
            // if(SG_Angle > 180) SG_Angle = 0;

            // SG_SET_Angle(SG_Angle);

            // printf("Angle:%d\r\n",SG_Angle);

            // LoRa_Send_string(1,"lora 13 14 run!");

            // printf("lora_get_flag:%d\r\n",lora_get_flag);

            // SG_SET_Angle(90);

           if(LCD_switch == 1) LoRa_Send_string(1,"afd");

            break;
        }
        
        default:
            break;
        }


        /* 睡眠1秒 */
        LOS_Msleep(20);
    }
}

void ALL_PERIPHERAL_INIT()
{   
    adc_dev_init();
    BEEP_Init();
    SG_Init();
    i2c_dev_init();
    mq2_dev_init();
    mq2_ppm_calibration();
    UART0_MO_Init();
}

void ALL_TASK_THREAD_INIT()
{   
    /*TASK_THREAD 变量*/
    unsigned int thread_id_1;
    unsigned int thread_id_2;
    unsigned int thread_id_3;
    unsigned int thread_id_4;
    unsigned int thread_id_5;
    unsigned int thread_id_6;
    unsigned int thread_id_7;
    unsigned int thread_id_8;
    unsigned int thread_id_9;

    TSK_INIT_PARAM_S task_1 = {0};
    TSK_INIT_PARAM_S task_2 = {0};
    TSK_INIT_PARAM_S task_3 = {0};
    TSK_INIT_PARAM_S task_4 = {0};
    TSK_INIT_PARAM_S task_5 = {0};
    TSK_INIT_PARAM_S task_6 = {0};
    TSK_INIT_PARAM_S task_7 = {0};
    TSK_INIT_PARAM_S task_8 = {0};
    TSK_INIT_PARAM_S task_9 = {0};

    unsigned int ret = LOS_OK;

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    ret = LOS_QueueCreate("mqtt_queue", MSG_QUEUE_LENGTH, &mqtt_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    ret = LOS_QueueCreate("su03_queue", MSG_QUEUE_LENGTH, &m_su03_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task_1.pfnTaskEntry = (TSK_ENTRY_FUNC)Debug_Test_thread;
    task_1.uwStackSize = 2048;
    task_1.pcName = "Debug_Test_thread";
    task_1.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id_1, &task_1);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }

    task_2.pfnTaskEntry = (TSK_ENTRY_FUNC)device_read_thread;
    task_2.uwStackSize = 2048;
    task_2.pcName = "device read thread";
    task_2.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id_2, &task_2);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }

    // task_3.pfnTaskEntry = (TSK_ENTRY_FUNC)body_process;
    // task_3.uwStackSize = 2048;
    // task_3.pcName = "body_process";
    // task_3.usTaskPrio = 24;
    // ret = LOS_TaskCreate(&thread_id_3, &task_3);
    // if (ret != LOS_OK)
    // {
    //     printf("Falied to create task ret:0x%x\n", ret);
    //     return;
    // }

    task_4.pfnTaskEntry = (TSK_ENTRY_FUNC)KEY_process;
    task_4.uwStackSize = 2048;
    task_4.pcName = "KEY_process";
    task_4.usTaskPrio = 23;
    ret = LOS_TaskCreate(&thread_id_4, &task_4);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }

    task_5.pfnTaskEntry = (TSK_ENTRY_FUNC)su_03t_thread;
    task_5.uwStackSize = 2048;
    task_5.pcName = "SU_03T";
    task_5.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id_5, &task_5);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }

    task_6.pfnTaskEntry = (TSK_ENTRY_FUNC)UART0_rev_Process;
    task_6.uwStackSize = 2048;
    task_6.pcName = "UART0_REV";
    task_6.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id_6, &task_6);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }

    task_7.pfnTaskEntry = (TSK_ENTRY_FUNC)wifi_sta_mode;
    task_7.uwStackSize = 8192;
    task_7.pcName = "wifi_sta_mode";
    task_7.usTaskPrio = 23;
    ret = LOS_TaskCreate(&thread_id_7, &task_7);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }

    task_8.pfnTaskEntry = (TSK_ENTRY_FUNC)mqtt_task;
    task_8.uwStackSize = 10240 + 1024;
    task_8.pcName = "mqtt_task";
    task_8.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id_8, &task_8);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }

    task_9.pfnTaskEntry = (TSK_ENTRY_FUNC)mqtt_send_process;
    task_9.uwStackSize = 8192 + 512;
    task_9.pcName = "mqtt_send_process";
    task_9.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id_9, &task_9);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

// void UART_TX_Init(void)
// {
//     IoTGpioInit(UART0_RX);
// 
//     IoTGpioSetDir(UART0_RX, IOT_GPIO_DIR_OUT);
// 
//     IoTGpioSetOutputVal(UART0_RX, IOT_GPIO_VALUE1);
// }

void Muetx_INIT(void)
{   
    unsigned int ret = LOS_OK;
    ret = LOS_MuxCreate(&m_mutex_UART0);
    if (ret != LOS_OK)
    {
        printf("Falied to create Mutex\n");
    }
    ret = LOS_MuxCreate(&m_mutex_WIFI);
    if (ret != LOS_OK)
    {
        printf("Falied to create Mutex\n");
    }
}

void Menu0(void)
{       
    uint16_t Line = 0;

    sprintf(LCD_BUF,"       Page1        ");
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF,"temp:%.2lf Deg C  ",lcd_temp);
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF,"humi:%.2lf RH    ",lcd_humi);
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF,"Light:%.2lf lx    ",lcd_dat);
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF,"CO:%.2lf ppm    ",lcd_gas);
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);
}

void Menu1(void)
{       
    uint16_t Line = 0;

    lcd_m1_weight = atof(a_weight); 
    lcd_m1_water  = atoi(a_water);

    // lcd_m1_weight = atof("200.00"); 
    // lcd_m1_water  = atoi("1999");

    sprintf(LCD_BUF,"       Page2        ");
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF, "M1 Food : %.1lf g    ", lcd_m1_weight); 
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF, "M1 Water: %d ADC    ", lcd_m1_water);
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF, "UP: to feed    ");
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    sprintf(LCD_BUF, "DOWN: to water   ");
    Line += 32;
    lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    // sprintf(LCD_BUF,"temp:%.2lf Deg C  ",lcd_temp);
    // Line += 32;
    // lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    // sprintf(LCD_BUF,"humi:%.2lf RH    ",lcd_humi);
    // Line += 32;
    // lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    // sprintf(LCD_BUF,"Light:%.2lf lx    ",lcd_dat);
    // Line += 32;
    // lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);

    // sprintf(LCD_BUF,"CO:%.2lf ppm    ",lcd_gas);
    // Line += 32;
    // lcd_show_string(0, Line, LCD_BUF, LCD_WHITE, LCD_BLACK, 32, 0);
}

void LCD_Process(void)
{

    uint32_t ret = 0;

    uint8_t last_switch = 0xFF;

    ret = lcd_init();
    if (ret != 0)
    {
        printf("lcd_init failed(%d)\n", ret);
        return;
    }
    lcd_fill(0, 0, LCD_W, LCD_H, LCD_BLACK);

    while(1)
    {   
        if (LCD_switch != last_switch)
        {
            lcd_fill(0, 0, LCD_W, LCD_H, LCD_BLACK); // 修正坐标
            last_switch = LCD_switch; // 更新状态
        }

        if(LCD_switch == 0) Menu0();
        
        else if(LCD_switch == 1) Menu1();

        LOS_Msleep(300);
    }

}

void LCD_Task(void)
{
    unsigned int thread_id_lcd;
    TSK_INIT_PARAM_S task_lcd = {0};
    unsigned int ret = LOS_OK;

    task_lcd.pfnTaskEntry = (TSK_ENTRY_FUNC)LCD_Process;
    task_lcd.uwStackSize = 4096;
    task_lcd.pcName = "LCD_Process";
    task_lcd.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id_lcd, &task_lcd);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

void LORA_GET_Process(void)
{
    while (1)
    {
        if(lora_get_flag == 1 || a_flag < 2)
        {
            /* 如果刚被MQTT触发(mqtt回调刚发了LoRa命令),
               等一等再发getd，避免模块来不及处理 */
            if (lora_get_flag == 1)
            {
                LOS_Msleep(LORA_CMD_GUARD_MS);
            }

            LoRa_Send_string(1, "getd");
            lora_get_flag = 0;
            LOS_Msleep(2000);
        }
        else
        {
            printf("a_water:%s\r\n", a_water);
            printf("a_weight:%s\r\n", a_weight);
            mqtt_publish_message((const char*)"rk2206/grain_1", (const char*)a_weight);
            LOS_Msleep(1000);
            mqtt_publish_message((const char*)"rk2206/water_1", (const char*)a_water);
            LOS_Msleep(1000);
            a_flag = 0;
        }
    }
}

void LORA_Task(void)
{
    unsigned int thread_id_lora;
    TSK_INIT_PARAM_S task_lora = {0};
    unsigned int ret = LOS_OK;

    task_lora.pfnTaskEntry = (TSK_ENTRY_FUNC)LORA_GET_Process;
    task_lora.uwStackSize = 2048;
    task_lora.pcName = "LORA_GET_Process";
    task_lora.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id_lora, &task_lora);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

void MAIN_VOID()
{
    printf("main\n");
    Muetx_INIT();
    UART0_MO_Init();
    Lora_Exit_AT();
    SG_Init();
    gpio_init();
    ALL_PERIPHERAL_INIT();
    LCD_Task();
    LORA_Task();
    ALL_TASK_THREAD_INIT();
    
    // LCD_Process();
    
}

APP_FEATURE_INIT(MAIN_VOID);

