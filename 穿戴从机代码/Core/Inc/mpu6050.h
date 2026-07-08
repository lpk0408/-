#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"
#include "i2c.h"

#define MPU6050_I2C_ADDR 0xD0
#define MPU6050_WHO_AM_I 0x75

extern unsigned long step_count;

void mpu6050_Init(void);
int mpu6050_read(uint8_t addr, uint8_t reg, uint8_t len, uint8_t* buf);
int mpu6050_write(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);
void mpu6050_read_and_print(void);

void mpu6050_set_step_distance(float cm);
unsigned long mpu6050_get_step_count(void);
void mpu6050_reset_step_count(void);
void mpu6050_step_detect_and_print(void);

#endif
