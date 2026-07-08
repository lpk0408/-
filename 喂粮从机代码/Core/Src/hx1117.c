#include "main.h"
#include "hx1117.h"
#include "delay.h"
#include "stdio.h"

int value;
float weight;
int32_t reset;
uint8_t buff[30];

float Weights = 200.0;          // 200g 酸奶
int32_t Weights_200 = 7852170;  // 放上酸奶后的稳定值

unsigned long HX711_GetData(void)
{
	unsigned long Count = 0;
	uint32_t timeout = 1000000;

	HX711_SCK_L;
	delay_us(1);

	while (HX711_DT) {
			if (--timeout == 0) {
//				printf("error\r\n");
				return 0xFFFFFFFF;   // 超时错误标识
			}
	}

	for (uint8_t i = 0; i < 24; i++) {
			HX711_SCK_H;
			delay_us(1);
			Count <<= 1;
			HX711_SCK_L;
			delay_us(1);
			if (HX711_DT) Count++;
	}

	HX711_SCK_H;
	delay_us(1);
	Count ^= 0x800000;
	HX711_SCK_L;
	delay_us(1);

	return Count;
}

void HX711_Init(void)
{		
	__HAL_RCC_GPIOA_CLK_ENABLE();
		
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /*Configure GPIO pin : PA0 */
	//HX711_SCK
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
	//HX711_DOUT
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_Delay(1000);
	
	reset = HX711_GetData();
	
//	printf("reset:%d\r\n",reset);
}

float HX711_Get_Weight(void)
{
	value = HX711_GetData();
	
	weight=(float)(value-reset)*Weights/(float)(Weights_200-reset);
		
	sprintf((char*)buff,"%.1f   ",weight);
	
//	printf("wight: %.1f\r\n",weight);
	
	return weight;
}




















