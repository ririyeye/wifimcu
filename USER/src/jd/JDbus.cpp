#include "JDbus.h"
#include "myuart.h"
#include "cmsis_os2.h"
#include "myuart.h"
#include "stdio.h"
#include "stm32f10x.h"
#include <string.h>

static uint64_t thread_main_stk_1[64];

void JD_main(void *argument);

void init_JD(void)
{
	static osThreadAttr_t thread1_attr;
	thread1_attr.stack_mem = &thread_main_stk_1[0];
	thread1_attr.stack_size = sizeof(thread_main_stk_1);
	osThreadId_t ost = osThreadNew(&JD_main, NULL, &thread1_attr);
}

static unsigned char txbuff[1024];
static unsigned char rxbuff[1024];

void JD_main(void *argument)
{
	UART_INFO *p3 = get_myuart(3);
	p3->opts->setspeed(230400);



    
}
