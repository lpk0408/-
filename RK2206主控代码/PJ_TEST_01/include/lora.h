#ifndef _LORA_H_
#define _LORA_H_

int Lora_Entry_AT(void);

int Lora_Exit_AT(void);

void LoRa_Send_double(double x);

void LoRa_Send_string(int index,char* str);

#endif
