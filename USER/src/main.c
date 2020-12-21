#include "stm32f10x.h"
#include <stdint.h>
#include "cmsis_os2.h"
#include "myuart.h"
#include <string.h>
#include "ec200/ec200.h"


int uart1_init(unsigned int bound);
int uart2_init(unsigned int bound);
int uart3_init(unsigned int bound);

void init_JD(void);

void app_main(void *argument)
{
	init_ec200_thread();

	while (true) {
		osDelay(1000);
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	uart1_init(115200);

	int osret = osKernelInitialize();

	osThreadId_t ost = osThreadNew(&app_main, NULL, NULL);

	osKernelStart();
}
