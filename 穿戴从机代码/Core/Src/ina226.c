#include "main.h"
#include "i2c.h"
#include "stdio.h"
#include "ina226.h"
#include "i2c_hal.h"

float P_pre = 0.0,P_cur = 0.0;

int P_Flag = 1;

double ina226_pwr = 0.0;

#define BAT_TOTAL_MWH 7400.0f

uint16_t INA226_ReadID(void)
{
    uint8_t high_byte = 0;
    uint8_t low_byte = 0;
    uint16_t id = 0;

    I2CStart();

    I2CSendByte(INA226_WRITE_ADDR);
    if (I2CWaitAck() != 0)
		{
        I2CStop();
        return 0;
    }
		
    I2CSendByte(INA226_REG_ID);
    if (I2CWaitAck() != 0) {
        I2CStop();
        return 0;
    }
		
    I2CStart();

    I2CSendByte(INA226_READ_ADDR);
    if (I2CWaitAck() != 0) {
        I2CStop();
        return 0;
    }

    high_byte = I2CReceiveByte();
    I2CSendAck(); 

    low_byte = I2CReceiveByte();
    I2CSendNotAck();
		
    I2CStop();

    id = (high_byte << 8) | low_byte;
    return id;
}

uint16_t INA226_ReadReg(uint8_t RegAddress)
{
    uint16_t Data = 0;
    uint8_t H, L;

    I2CStart();
    I2CSendByte(INA226_WRITE_ADDR);
    if(I2CWaitAck()) { I2CStop(); return 0; }

    I2CSendByte(RegAddress);
    if(I2CWaitAck()) { I2CStop(); return 0; }

    I2CStart();
    I2CSendByte(INA226_READ_ADDR);
    if(I2CWaitAck()) { I2CStop(); return 0; }

    H = I2CReceiveByte();
    I2CSendAck();

    L = I2CReceiveByte();
    I2CSendNotAck();

    I2CStop();

    Data = (H << 8) | L;
    return Data;
}


void INA226_WriteReg(uint8_t reg_addr, uint16_t value)
{
    I2CStart();
    I2CSendByte(INA226_WRITE_ADDR);
    if(I2CWaitAck() != 0) { I2CStop(); return; }
    
    I2CSendByte(reg_addr); 
    if(I2CWaitAck() != 0) { I2CStop(); return; }
    
    I2CSendByte((value >> 8) & 0xFF);
    if(I2CWaitAck() != 0) { I2CStop(); return; }
    
    I2CSendByte(value & 0xFF);
    if(I2CWaitAck() != 0) { I2CStop(); return; }
    
    I2CStop();
}

void INA226_Init(void)
{
	
	uint16_t target_id = INA226_ReadID();

	if (target_id == 0x2260) {
			printf("Software I2C Read OK! ID: 0x%04X\r\n", target_id);
	} else if (target_id == 0) {
			printf("Software I2C No Ack! Check your GPIO pins or Pull-up resistors.\r\n");
	} else {
			printf("Software I2C Communication active, but wrong ID: 0x%04X\r\n", target_id);
	}
	
	INA226_WriteReg(0x00,0x4527); 
	
	INA226_WriteReg(0x05,0xA00);
}

uint16_t INA226_GetShuntVoltage(void)//????? =  ???? * LSB(2.5uA)
{
	uint16_t ShuntVoltage;
	ShuntVoltage = (uint16_t)((INA226_ReadReg(0x01)));
	
	return ShuntVoltage;
}

uint16_t INA226_GetBusVoltage(void)
{
	uint16_t BusVoltage;
	BusVoltage = (uint16_t)((INA226_ReadReg(0x02)) * 1.25);
	
	return BusVoltage;
}


uint16_t INA226_GetCurrent(void)
{
	uint16_t Current;
	Current = (uint16_t)(INA226_ReadReg(0x04));
	
	return Current;
}


uint16_t INA226_GetPower(void)//?? = ???? * Power_LSB(1.25mW)
{
	uint32_t Power;
	Power = (uint16_t)(INA226_ReadReg(0x03));
	
	return Power;
}

float ina226_get_cur(void)
{
	return INA226_GetCurrent() * 0.02;
}

float ina226_get_pow(void)
{
	return INA226_GetPower() * 0.5;
}

double ina226_get_bat(void)
{
	if(P_Flag)
	{
		P_pre = ina226_get_pow();
		
		P_Flag = 0;
	}
	
	P_cur = ina226_get_pow();
	
	ina226_pwr += (P_pre + P_cur) * 0.1 * 0.5;
	
	P_pre = P_cur;
	
	return ina226_pwr;
}


//float get_bat_soc(void)
//{
//	double temp = ina226_get_bat();

//	double consume_mwh = ina226_pwr / 3600.0f;
//	double remain_mwh = BAT_TOTAL_MWH - consume_mwh;
//	if(remain_mwh < 0) remain_mwh = 0;
//	return remain_mwh / BAT_TOTAL_MWH * 100.0f;
//}

float soc = 0.0;

float get_bat_soc(void)
{
    int vol = INA226_GetBusVoltage(); 
    
		if(vol >= 4200) soc = 100.0;
		else if(vol >= 3800) soc = 80.0;
		else if(vol >= 3400) soc = 60.0;
		else if(vol >= 3000) soc = 40.0;
		
		return soc;
}

