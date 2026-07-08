#ifndef _GPS_H_
#define _GPS_H_

#include <stdint.h>

// GPS状态定义
#define GPS_OFFLINE     0   // GPS未启动，无数据
#define GPS_STARTING    1   // GPS已启动，正在冷启动/搜星中（收到NMEA数据但无定位）
#define GPS_FIXED       2   // GPS已定位成功

extern uint8_t GPS_Status;
extern uint8_t GPS_Antenna_OK;

void Get_NMEA_Field(const char *line, int field_index, char *output);
uint8_t GPS_GetStatus(void);
const char* GPS_GetStatusString(void);

#endif
