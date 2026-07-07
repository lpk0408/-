#ifndef __DATA_BASIC_H__
#define __DATA_BASIC_H__


void i2c_dev_init(void);
void bh1750_read_data(double *dat);
void sht30_read_data(double *temp, double *humi);

extern unsigned int mq2_dev_init(void);
extern void mq2_ppm_calibration(void);
extern float get_mq2_ppm(void);

// // 在调用前加入明确的原型声明
// float get_mq2_ppm(void);
// void mq2_ppm_calibration(void);
// unsigned int mq2_dev_init(void);

#endif
