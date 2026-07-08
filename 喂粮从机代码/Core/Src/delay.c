#include "delay.h"
#include "main.h"
#include "stdio.h"

uint32_t fac_us = 64;

void DWT_Delay_Init(void)
{
    // 1. 启用 CoreDebug 模块中的跟踪功能 (TRCENA)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // 2. 清零周期计数器
    DWT->CYCCNT = 0;
    // 3. 启用 DWT 周期计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// 极速、精准的微秒延时函数
void delay_us(uint32_t nus)
{
    uint32_t startTick = DWT->CYCCNT;      // 记录开始时的周期计数
    uint32_t delayTicks = nus * fac_us;    // 计算需要等待的总周期数

    // 循环等待，直到走过的周期数达到要求
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

void delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}

uint32_t get_ms(void)
{
	return HAL_GetTick();
}

void get_ms_t(unsigned long *count)
{
    *count = HAL_GetTick();
}

//void delay_us(uint32_t nus)
//{
//    uint32_t startTick = DWT->CYCCNT;
//    uint32_t delayTicks = nus * 64; // 64MHz 对应 fac_us=64

//    // 启用 DWT 计数器 (部分内核需要先使能)
//    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
//        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
//        DWT->CYCCNT = 0;
//        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
//    }

//    while ((DWT->CYCCNT - startTick) < delayTicks);
//}

void Test_Delay_us(void)
{
    uint32_t start_tick, end_tick, tick_diff;
    uint32_t actual_us;

    // 1. 记录延时前的 TICK 值
    start_tick = DWT->CYCCNT;

    // 2. 调用你的延时函数（比如延时 100 微秒）
    delay_us(100);

    // 3. 记录延时后的 TICK 值
    end_tick = DWT->CYCCNT;

    // 4. 计算经过的滴答数（利用无符号减法，自动处理溢出）
    tick_diff = end_tick - start_tick;

    // 5. 换算成微秒（64MHz下，除以64即可）
    actual_us = tick_diff / 64; 

    // 6. 打印结果（注意：printf 必须放在所有计时结束之后！）
    printf("Target Delay: 100 us\r\n");
    printf("Tick Diff: %u \r\n", tick_diff);
    printf("Actual Delay: %u us\r\n", actual_us);
}














