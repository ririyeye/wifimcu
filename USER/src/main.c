#include "stm32f10x.h"
#include <stdint.h>
#include "cmsis_os2.h"
#include "myuart.h"
#include <string.h>
#include <sbwt.h>
#include "DMT.h"
#include "ebyte_lora.h"
#include "cc3200/cc3200.h"
#include "wifi/wifi_par.h"

int uart1_init(unsigned int bound);
int uart2_init(unsigned int bound);
int uart3_init(unsigned int bound);

void init_JD(void);
void work_led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //开启U2串口时钟

	//com io
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//live io
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void set_live_led(int sta)
{
	if (sta) {
		GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);
	} else {
		GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);
	}
}

void set_com_led(int sta)
{
	if (sta) {
		GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET);
	} else {
		GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET);
	}
}

void app_main(void *argument)
{
	init_cc3200();
	init_JD();
	timupdate_init();
	work_led_init();
	set_live_led(0);
	while (true) {
		osDelay(1000);
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	uart1_init(115200);
	uart2_init(115200);
	uart3_init(230400);

	int osret = osKernelInitialize();

	osThreadId_t ost = osThreadNew(&app_main, NULL, NULL);

	osKernelStart();
}
