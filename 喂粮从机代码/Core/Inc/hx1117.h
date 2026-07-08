#include "main.h"

#define 	HX711_SCK_GPIO_PORT							GPIOA
#define 	HX711_SCK_GPIO_PIN							GPIO_PIN_0

#define 	HX711_DT_GPIO_PORT							GPIOA
#define 	HX711_DT_GPIO_PIN								GPIO_PIN_1

#define 	HX711_SCK_H											HAL_GPIO_WritePin(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN, GPIO_PIN_SET);
#define 	HX711_SCK_L											HAL_GPIO_WritePin(HX711_SCK_GPIO_PORT, HX711_SCK_GPIO_PIN, GPIO_PIN_RESET);

#define 	HX711_DT												HAL_GPIO_ReadPin(HX711_DT_GPIO_PORT,HX711_DT_GPIO_PIN)


void HX711_Init(void);

unsigned long HX711_GetData(void);

float HX711_Get_Weight(void);

