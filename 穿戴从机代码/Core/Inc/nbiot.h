#ifndef _NBIOT_H_
#define _NBIOT_H_

void NBIOT_CIMI(void);

void NBIOT_MQTT_Client_Create(void);

void NBIOT_MQTT_Client_Connect(void);

void NBIOT_MQTT_Client_Connect(void);

void NBIOT_MQTT_Send_Message(char* tpoic,char* message);

void StringToHexStr(char* src, char* dest, int max_len);

#endif
