
#include "all.h"

void mqtt_task(void *args);


int mqtt_publish_message(const char *topic, const char *payload);

void wifi_sta_mode();

int mqtt_publish_message(const char *topic, const char *payload);


extern uint8_t lora_get_flag;
