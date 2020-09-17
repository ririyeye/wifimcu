#include "cmsis_os2.h"
#include "myuart.h"
#include "stdio.h"
#include "stm32f10x.h"
#include <string.h>

template <typename T, size_t N> inline size_t CountOf(T (&arr)[N])
{
	return N;
}

static uint64_t thread_main_stk_1[64];

void cc3200_main(void *argument);

void init_cc3200()
{
	static osThreadAttr_t thread1_attr;
	thread1_attr.stack_mem = &thread_main_stk_1[0];
	thread1_attr.stack_size = sizeof(thread_main_stk_1);
	osThreadId_t ost = osThreadNew(&cc3200_main, NULL, &thread1_attr);
}

static unsigned char txbuff[1024];
static unsigned char rxbuff[1024];

void init_cc3200_gpio()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //开启U2串口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_SET);
}

int speeds[] = { 115200 * 1, 115200 * 4 };

int checkATmod(char *rxbuff, int size)
{
	if (strstr(rxbuff, "Entered AT")) {
		return 0;
	}

	if (strstr(rxbuff, "Unable to")) {
		return 0;
	}
	return -1;
}

int enter_at_mode(UART_INFO *info)
{
	int txnum = sprintf((char *)txbuff, "+++");

	for (int retrycnt = 0; retrycnt < 100; retrycnt++) {
		info->opts->setspeed(speeds[retrycnt % CountOf(speeds)]);

		osDelay(100);

		info->send(txbuff, txnum);
		info->rece(rxbuff, sizeof(rxbuff));
		info->wait_rece(200, 200);

		if (info->GetRxNum() > 0 && !checkATmod((char *)rxbuff, info->GetRxNum())) {
			return 0;
		}
	}
	return -1;
}

void cc3200_main(void *argument)
{
	init_cc3200_gpio();

	UART_INFO *p1 = get_myuart(1);

	enter_at_mode(p1);

	while (true) {
		osDelay(10);
	}
}
