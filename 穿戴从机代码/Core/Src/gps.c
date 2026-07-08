#include "gps.h"
#include "all_include.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

extern UINT32 g_uart2_rx_sem;

// GPS全局状态变量
uint8_t GPS_Status = GPS_OFFLINE;
uint8_t GPS_Antenna_OK = 0;

void Get_NMEA_Field(const char *line, int field_index, char *output)
{
    int current_field = 0;
    const char *p = line;

    // 1. 找到对应的逗号位置
    while (current_field < field_index && *p != '\0') {
        if (*p == ',') {
            current_field++;
        }
        p++;
    }

    // 2. 拷贝数据直到下一个逗号或字符串结束符
    int i = 0;
    while (*p != ',' && *p != '\0' && *p != '*' && *p != '\r' && *p != '\n') {
        output[i++] = *p++;
    }
    output[i] = '\0'; // 添加字符串结束符
}

uint8_t GPS_GetStatus(void)
{
    return GPS_Status;
}

const char* GPS_GetStatusString(void)
{
    switch(GPS_Status)
    {
        case GPS_OFFLINE:  return "离线";
        case GPS_STARTING: return "搜星中";
        case GPS_FIXED:    return "已定位";
        default:           return "未知";
    }
}
