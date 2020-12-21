#include "ec200/ec200.h"
#include "cmsis_os2.h"
#include "myuart.h"
#include "stdio.h"
#include "stm32f10x.h"
#include <string.h>

static uint64_t thread_cc3200_stk[64];
static unsigned char txbuff[1024];
static unsigned char rxbuff[1024];

template <typename T, size_t N> inline size_t CountOf(T (&arr)[N])
{
	return N;
}


void ec200_main(void *argument);

void init_ec200_thread(void)
{
	osThreadAttr_t thread1_attr;
	memset(&thread1_attr, 0, sizeof(osThreadAttr_t));
	thread1_attr.stack_mem = &thread_cc3200_stk[0];
	thread1_attr.stack_size = sizeof(thread_cc3200_stk);
	thread1_attr.priority = osPriorityHigh1;
	osThreadId_t ost = osThreadNew(&ec200_main, NULL, &thread1_attr);
}

int getcsq(UART_INFO *info)
{
	char *cmd = "AT+CSQ\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, 10);

	int rxnum = info->GetRxNum();
	char * pos = strstr((char *)rxbuff, "+CSQ: ");
	if (rxnum && pos) {
		int dat = 0;
		int dat2 = 0;
		if(2 == sscanf(pos, "+CSQ: %d,%d", &dat, &dat2)){
			return dat;
		}
	}
	return -1;
}

void ec200_main(void *argument)
{
	UART_INFO *pec200 = get_myuart(1);
	while (1) {
		getcsq(pec200);
		osDelay(1000);
	}
}
