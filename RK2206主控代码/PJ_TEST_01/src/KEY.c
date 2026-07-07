#include "all.h"

/* 按键对应ADC通道 */
#define KEY_ADC_CHANNEL 7

/***************************************************************
* 函数名称: adc_dev_init
* 说    明: 初始化ADC
* 参    数: 无
* 返 回 值: 0为成功，反之为失败
***************************************************************/
unsigned int adc_dev_init()
{
    unsigned int ret = 0;

    /* 初始化ADC */
    ret = IoTAdcInit(KEY_ADC_CHANNEL);

    if(ret != IOT_SUCCESS)
    {
        printf("%s, %s, %d: ADC Init fail\n", __FILE__, __func__, __LINE__);
    }

    return 0;
}

/***************************************************************
* 函数名称: Get_Voltage
* 说    明: 获取ADC电压值
* 参    数: 无
* 返 回 值: 电压值
***************************************************************/
float adc_get_voltage()
{
    unsigned int ret = IOT_SUCCESS;
    unsigned int data = 0;

    /* 获取ADC值 */
    ret = IoTAdcGetVal(KEY_ADC_CHANNEL, &data);

    if (ret != IOT_SUCCESS)
    {
        printf("%s, %s, %d: ADC Read Fail\n", __FILE__, __func__, __LINE__);
        return 0.0;
    }

    return (float)(data * 3.3 / 1024.0);
}




