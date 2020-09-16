#include "cmsis_os2.h"
#include "myuart.h"


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



void cc3200_main(void *argument)
{
	UART_INFO * p1 = get_myuart(1);



}
