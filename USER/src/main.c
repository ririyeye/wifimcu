#include "stm32f10x.h"
#include <stdint.h>
#include "cmsis_os2.h"
#include "myuart.h"
#include <string.h>
#include <sbwt.h>
#include "DMT.h"
#include "ebyte_lora.h"
void uart1_init(unsigned int bound);
void uart2_init(unsigned int bound);
void uart3_init(unsigned int bound);
void uart4_init(unsigned int bound);
void uart5_init(unsigned int bound);

void init_sbwt();
void init_display();
void init_lora();
void init_sbwt_modbus();

void app_main(void *argument)
{
	init_sbwt();
	init_display();
	init_ebyte_lora();
	init_sbwt_modbus();

	while (true) {
		osDelay(1000);
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	uart1_init(115200);
	uart2_init(115200);
	uart3_init(19200);

	uart4_init(9600);

	uart5_init(9600);

	int osret = osKernelInitialize();

	osThreadId_t ost = osThreadNew(&app_main, NULL, NULL);

	osKernelStart();
}
