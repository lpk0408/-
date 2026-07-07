#include "DATA_Basic.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "iot_errno.h"
#include "iot_i2c.h"
#include "iot_adc.h"


#define I2C_HANDLE EI2C0_M2

#define SHT30_I2C_ADDRESS 0x44
#define BH1750_I2C_ADDRESS 0x23

static uint32_t sht30_init(void)
{
    uint32_t ret = 0;
    uint8_t send_data[2] = {0x22, 0x36};
    uint32_t send_len = 2;

    ret = IoTI2cWrite(I2C_HANDLE, SHT30_I2C_ADDRESS, send_data, send_len); 
    if (ret != IOT_SUCCESS)
    {
        printf("I2c write failure.\r\n");
        return IOT_FAILURE;
    }

    return IOT_SUCCESS;
}

static uint32_t bh1750_init(void)
{
    uint32_t ret = 0;
    uint8_t send_data[1] = {0x10};
    uint32_t send_len = 1;

    ret = IoTI2cWrite(I2C_HANDLE, SHT30_I2C_ADDRESS, send_data, send_len); 
    if (ret != IOT_SUCCESS)
    {
        printf("I2c write failure.\r\n");
        return IOT_FAILURE;
    }

    return IOT_SUCCESS;
}

static float sht30_calc_RH(uint16_t u16sRH)
{
    float humidityRH = 0;

    /*clear bits [1..0] (status bits)*/
    u16sRH &= ~0x0003;
    /*calculate relative humidity [%RH]*/
    /*RH = rawValue / (2^16-1) * 10*/
    humidityRH = (100 * (float)u16sRH / 65535);

    return humidityRH;
}

static float sht30_calc_temperature(uint16_t u16sT)
{
    float temperature = 0;

    /*clear bits [1..0] (status bits)*/
    u16sT &= ~0x0003;
    /*calculate temperature [℃]*/
    /*T = -45 + 175 * rawValue / (2^16-1)*/
    temperature = (175 * (float)u16sT / 65535 - 45);

    return temperature;
}

static uint8_t sht30_check_crc(uint8_t *data, uint8_t nbrOfBytes, uint8_t checksum)
{
    uint8_t crc = 0xFF;
    uint8_t bit = 0;
    uint8_t byteCtr ;
    const int16_t POLYNOMIAL = 0x131;

    /*calculates 8-Bit checksum with given polynomial*/
    for(byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for ( bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else crc = (crc << 1);
        }
    }

    if(crc != checksum)
        return 1;
    else
        return 0;
}

void sht30_read_data(double *temp, double *humi)
{
    /*checksum verification*/
    uint8_t data[3];
    uint16_t tmp;
    uint8_t rc;
    /*byte 0,1 is temperature byte 4,5 is humidity*/
    uint8_t SHT30_Data_Buffer[6];
    memset(SHT30_Data_Buffer, 0, 6);
    uint8_t send_data[2] = {0xE0, 0x00};

    uint32_t send_len = 2;
    IoTI2cWrite(I2C_HANDLE, SHT30_I2C_ADDRESS, send_data, send_len);

    uint32_t receive_len = 6;
    IoTI2cRead(I2C_HANDLE, SHT30_I2C_ADDRESS, SHT30_Data_Buffer, receive_len);

    /*check temperature*/
    data[0] = SHT30_Data_Buffer[0];
    data[1] = SHT30_Data_Buffer[1];
    data[2] = SHT30_Data_Buffer[2];
    rc = sht30_check_crc(data, 2, data[2]);
    if(!rc)
    {
        tmp = ((uint16_t)data[0] << 8) | data[1];
        *temp = sht30_calc_temperature(tmp);
    }
    
    /*check humidity*/
    data[0] = SHT30_Data_Buffer[3];
    data[1] = SHT30_Data_Buffer[4];
    data[2] = SHT30_Data_Buffer[5];
    rc = sht30_check_crc(data, 2, data[2]);
    if(!rc)
    {
        tmp = ((uint16_t)data[0] << 8) | data[1];
        *humi = sht30_calc_RH(tmp);
    }
}

void bh1750_read_data(double *dat)
{
    uint8_t send_data[1] = {0x10};
    uint32_t send_len = 1;

    IoTI2cWrite(I2C_HANDLE, BH1750_I2C_ADDRESS, send_data, send_len); 

    uint8_t recv_data[2] = {0};
    uint32_t receive_len = 2;   

    IoTI2cRead(I2C_HANDLE, BH1750_I2C_ADDRESS, recv_data, receive_len);
    *dat = (float)(((recv_data[0] << 8) + recv_data[1]) / 1.2);
}

void i2c_dev_init(void)
{
    IoTI2cInit(I2C_HANDLE, EI2C_FRE_400K);
    sht30_init();
    bh1750_init();
}

#define CAL_PPM 20 // 校准环境中PPM值
#define RL 1       // RL阻值

static float m_r0; // 元件在干净空气中的阻值

#define MQ2_ADC_CHANNEL 4

/***************************************************************
* 函数名称: mq2_dev_init
* 说    明: 初始化ADC
* 参    数: 无
* 返 回 值: 0为成功，反之为失败
***************************************************************/
unsigned int mq2_dev_init(void)
{
    unsigned int ret = 0;

    ret = IoTAdcInit(MQ2_ADC_CHANNEL);

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
static float adc_get_voltage(void)
{
    unsigned int ret = IOT_SUCCESS;
    unsigned int data = 0;

    ret = IoTAdcGetVal(MQ2_ADC_CHANNEL, &data);

    if (ret != IOT_SUCCESS)
    {
        printf("%s, %s, %d: ADC Read Fail\n", __FILE__, __func__, __LINE__);
        return 0.0;
    }

    return (float)(data * 3.3 / 1024.0);
}

/***************************************************************
 * 函数名称: mq2_ppm_calibration
 * 说    明: 传感器校准函数
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void mq2_ppm_calibration(void) 
{
  float voltage = adc_get_voltage();
  float rs = (5 - voltage) / voltage * RL;

  m_r0 = rs / powf(CAL_PPM / 613.9f, 1 / -2.074f);
}

/***************************************************************
 * 函数名称: get_mq2_ppm
 * 说    明: 获取PPM函数
 * 参    数: 无
 * 返 回 值: ppm
 ***************************************************************/
float get_mq2_ppm(void) 
{
  float voltage, rs, ppm;

  voltage = adc_get_voltage();
  rs = (5 - voltage) / voltage * RL;      // 计算rs
  ppm = 613.9f * powf(rs / m_r0, -2.074f); // 计算ppm
  return ppm;
}

