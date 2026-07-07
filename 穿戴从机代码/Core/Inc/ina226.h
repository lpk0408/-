#include "main.h"


uint16_t INA226_ReadReg(uint8_t RegAddress);

void INA226_WriteReg(uint8_t reg_addr, uint16_t value);

#define	INA226_Configuration                          0x00
#define INA226_Shuntvoltage                           0x01
#define INA226_Busvoltage                             0x02
#define INA226_Power                                  0x03
#define INA226_Current                                0x04
#define INA226_Calibration                            0x05
#define	INA226_Mask                                   0x06
#define	INA226_AlertLimit                             0x07
#define	INA226_ManufacturerID                         0xFE
#define	INA226_DieID                                  0xFF

//#define INA226_WRITE_ADDR   0x88  
//#define INA226_READ_ADDR    0x89  

//#define INA226_WRITE_ADDR   0x94  
//#define INA226_READ_ADDR    0x95 

#define INA226_WRITE_ADDR   0x8C
#define INA226_READ_ADDR    0x8D

#define INA226_REG_ID       0xFF

#define RST 			        0 		
#define Reservation       0x04  // 100 (Bit14-12 ??)
#define AVG 							0x01  // 001 ???? 4 (Bit11-9)
#define VBUSCT  					0x04	// 100 ???????? 1.1ms (Bit116-8)
#define VSHCT		  				0x04	// 100 ???????? 1.1ms(Bit3-5)
#define	MODE	            0x07 	// 111 ????  ????(??)(Bit0–2)

//#define Configuration_H 	(RST << 7)|(Reservation << 4)|(AVG << 1)|(VBUSCT >> 2)
//#define Configuration_L 	((VBUSCT & 0x03) << 6)|(VSHCT << 3)|(MODE)


#define Configuration_H 	0x43
#define Configuration_L 	0x27

#define Calibration_H 0x04
#define Calibration_L 0x00

void INA226_Init(void);

uint16_t INA226_GetShuntVoltage(void);

uint16_t INA226_GetBusVoltage(void);

uint16_t INA226_GetCurrent(void);

uint16_t INA226_GetPower(void);

float ina226_get_cur(void);

float ina226_get_pow(void);

double ina226_get_bat(void);

float get_bat_soc(void);






