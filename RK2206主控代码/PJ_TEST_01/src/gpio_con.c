#include "los_task.h"
#include "ohos_init.h"

#include "iot_errno.h"
#include "iot_gpio.h"
#include "gpio_con.h"


#define GPIO_FAN    GPIO0_PA5

#define GPIO_HUMI   GPIO0_PB4

// #define LCD_PIN_CS          GPIO0_PC0
// #define LCD_PIN_CLK         GPIO0_PC1
// #define LCD_PIN_MOSI        GPIO0_PC2
// #define LCD_PIN_RES         GPIO0_PC3
// #define LCD_PIN_DC          GPIO0_PA4

void gpio_init(void)
{
    IoTGpioInit(GPIO_FAN);
    IoTGpioInit(GPIO_HUMI);
}

void GPIO_FAN_ENABLE(void)
{
    IoTGpioSetDir(GPIO_FAN, IOT_GPIO_DIR_OUT);

    IoTGpioSetOutputVal(GPIO_FAN, IOT_GPIO_VALUE1);
}

void GPIO_FAN_DISABLE(void)
{
    IoTGpioSetDir(GPIO_FAN, IOT_GPIO_DIR_OUT);

    IoTGpioSetOutputVal(GPIO_FAN, IOT_GPIO_VALUE0);
}

void GPIO_HUMI_ENABLE(void)
{
    // IoTGpioInit(GPIO_HUMI);
    
    IoTGpioSetDir(GPIO_HUMI, IOT_GPIO_DIR_OUT);

    IoTGpioSetOutputVal(GPIO_HUMI, IOT_GPIO_VALUE1);
}

void GPIO_HUMI_DISABLE(void)
{
    // IoTGpioInit(GPIO_HUMI);
    
    IoTGpioSetDir(GPIO_HUMI, IOT_GPIO_DIR_OUT);

    IoTGpioSetOutputVal(GPIO_HUMI, IOT_GPIO_VALUE0);
}
