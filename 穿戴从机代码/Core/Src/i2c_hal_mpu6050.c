/**
 * CT117E-M4 / GPIO - I2C (STM32F4)
*/

#include "i2c_hal_mpu6050.h"

#define DELAY_TIME_mp      20

#define I2C_PORT_mp        GPIOB
#define I2C_SCL_PIN_mp     GPIO_PIN_8
#define I2C_SDA_PIN_mp     GPIO_PIN_9

void SDA_Input_Mode_mp(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = I2C_SDA_PIN_mp;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_PORT_mp, &GPIO_InitStructure);
}

void SDA_Output_Mode_mp(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = I2C_SDA_PIN_mp;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_PORT_mp, &GPIO_InitStructure);
}

// SDA????
void SDA_Output_mp( uint16_t val )
{
    if ( val )
    {
        I2C_PORT_mp->BSRR = I2C_SDA_PIN_mp;        // ?16?:????
    }
    else
    {
        I2C_PORT_mp->BSRR = (uint32_t)I2C_SDA_PIN_mp << 16; // F4?16?:????
    }
}

// SCL????
void SCL_Output_mp( uint16_t val )
{
    if ( val )
    {
        I2C_PORT_mp->BSRR = I2C_SCL_PIN_mp;        // ?16?:????
    }
    else
    {
        I2C_PORT_mp->BSRR = (uint32_t)I2C_SCL_PIN_mp << 16; // F4?16?:????
    }
}

// ??SDA??
uint8_t SDA_Input_mp(void)
{
    if(HAL_GPIO_ReadPin(I2C_PORT_mp, I2C_SDA_PIN_mp) == GPIO_PIN_SET){
        return 1;
    }else{
        return 0;
    }
}

static void delay1_mp(volatile unsigned int n)
{
    volatile uint32_t i;
    for ( i = 0; i < n; ++i);
}

// I2C????
void I2CStart_mp(void)
{
    SDA_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
    SDA_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
}

// I2C????
void I2CStop_mp(void)
{
    SCL_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
    SDA_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
    SDA_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
}

unsigned char I2CWaitAck_mp(void)
{
    unsigned short cErrTime = 5;
    SDA_Input_Mode_mp();
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
    while(SDA_Input_mp())
    {
        cErrTime--;
        delay1_mp(DELAY_TIME_mp);
        if (0 == cErrTime)
        {
            SDA_Output_Mode_mp();
            I2CStop_mp();
            return 0; // ????? ERROR (???0)
        }
    }
    SCL_Output_mp(0);
    SDA_Output_Mode_mp();
    delay1_mp(DELAY_TIME_mp);
    return 1; // ????? SUCCESS (???1)
}

void I2CSendAck_mp(void)
{
    SDA_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
}

void I2CSendNotAck_mp(void)
{
    SDA_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(1);
    delay1_mp(DELAY_TIME_mp);
    SCL_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
}

void I2CSendByte_mp(unsigned char cSendByte)
{
    unsigned char  i = 8;
    while (i--)
    {
        SCL_Output_mp(0);
        delay1_mp(DELAY_TIME_mp);
        SDA_Output_mp(cSendByte & 0x80);
        delay1_mp(DELAY_TIME_mp);
        cSendByte += cSendByte;
        delay1_mp(DELAY_TIME_mp);
        SCL_Output_mp(1);
        delay1_mp(DELAY_TIME_mp);
    }
    SCL_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
}

unsigned char I2CReceiveByte_mp(void)
{
    unsigned char i = 8;
    unsigned char cR_Byte = 0;
    SDA_Input_Mode_mp();
    while (i--)
    {
        cR_Byte += cR_Byte;
        SCL_Output_mp(0);
        delay1_mp(DELAY_TIME_mp);
        delay1_mp(DELAY_TIME_mp);
        SCL_Output_mp(1);
        delay1_mp(DELAY_TIME_mp);
        cR_Byte |=  SDA_Input_mp();
    }
    SCL_Output_mp(0);
    delay1_mp(DELAY_TIME_mp);
    SDA_Output_Mode_mp();
    return cR_Byte;
}

// I2C???
void I2CInit_mp(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.Pin = I2C_SCL_PIN_mp | I2C_SDA_PIN_mp;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(I2C_PORT_mp, &GPIO_InitStructure);
}