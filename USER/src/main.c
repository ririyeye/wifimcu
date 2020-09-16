#include "stm32f10x.h"
#include <stdint.h>
#include "cmsis_os2.h"
#include "myuart.h"
#include <string.h>
#include <sbwt.h>
#include "DMT.h"
#include "ebyte_lora.h"
int uart1_init(unsigned int bound);
int uart2_init(unsigned int bound);
int uart3_init(unsigned int bound);


void init_sbwt();
void init_display();
void init_lora();
void init_sbwt_modbus();
void init_cc3200();

void app_main(void *argument)
{
	// init_sbwt();
	// init_display();
	// init_ebyte_lora();
	// init_sbwt_modbus();
	init_cc3200();

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

	int osret = osKernelInitialize();

	osThreadId_t ost = osThreadNew(&app_main, NULL, NULL);

	osKernelStart();
}
