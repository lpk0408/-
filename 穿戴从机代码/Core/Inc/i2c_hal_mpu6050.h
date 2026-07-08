#ifndef __I2C_HAL_MPU6050_H
#define __I2C_HAL_MPU6050_H

#include "stm32f4xx_hal.h"

/* ?????(??????? SUCCESS/ERROR) */
#ifndef SUCCESS
#define SUCCESS  1
#endif

#ifndef ERROR
#define ERROR    0
#endif

/* ???? */
void SDA_Input_Mode_mp(void);
void SDA_Output_Mode_mp(void);
void SDA_Output_mp(uint16_t val);
void SCL_Output_mp(uint16_t val);
uint8_t SDA_Input_mp(void);

void I2CInit_mp(void);
void I2CStart_mp(void);
void I2CStop_mp(void);
void I2CSendAck_mp(void);
void I2CSendNotAck_mp(void);
unsigned char I2CWaitAck_mp(void);
void I2CSendByte_mp(unsigned char cSendByte);
unsigned char I2CReceiveByte_mp(void);

#endif /* __I2C_HAL_MPU6050_H */
