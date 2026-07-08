#include "all.h"

#define UART_HANDLE EUART0_M0

void UART0_MO_Init(void);

void UART0_M0_Print(const char *buf);

void UART0_TEST(void);

void UART0_rev_Process(void);

void UART0_M0_WriteRaw(const uint8_t *buf, size_t len);

extern char a_weight[64];

extern char a_water[64];

extern uint8_t a_finish_flag;

extern volatile uint8_t a_flag;

#define UART0_RX GPIO0_PB6

#define UART0_TX GPIO0_PB7
