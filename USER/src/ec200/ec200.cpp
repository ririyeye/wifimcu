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

int getCSQ(UART_INFO *info)
{
	char *cmd = "AT+CSQ\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	int rxnum = info->GetRxNum();
	char *pos = strstr((char *)rxbuff, "+CSQ: ");
	if (rxnum && pos) {
		int dat = 0;
		int dat2 = 0;
		if (2 == sscanf(pos, "+CSQ: %d,%d", &dat, &dat2)) {
			return dat;
		}
	}
	return -1;
}

int getCPIN(UART_INFO *info)
{
	char *cmd = "AT+CPIN?\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	int rxnum = info->GetRxNum();
	char *pos = strstr((char *)rxbuff, "+CPIN: READY");
	if (rxnum && pos) {
		return 0;
	}
	return -1;
}

int getCREG(UART_INFO *info)
{
	char *cmd = "AT+CREG?\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	int rxnum = info->GetRxNum();
	char *pos = strstr((char *)rxbuff, "+CREG: ");
	if (rxnum && pos) {
		return 0;
	}
	return -1;
}

int getCGATT(UART_INFO *info)
{
	char *cmd = "AT+CGATT?\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	int rxnum = info->GetRxNum();
	char *pos = strstr((char *)rxbuff, "+CGATT: ");
	if (rxnum && pos) {
		return 0;
	}
	return -1;
}

int getQICSGP(UART_INFO *info)
{
	char *cmd = "AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r\n";
	info->send(cmd, strlen(cmd));

	for (int i = 0; i < 5; i++) {
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, DEFAUTL_ACK_TIM);

		int rxnum = info->GetRxNum();
		char *pos = strstr((char *)rxbuff, "OK");
		if (rxnum && pos) {
			return 0;
		}

		pos = strstr((char *)rxbuff, "FAIL");
		if (rxnum && pos) {
			return -1;
		}
	}
	return -1;
}

int getQIACT(UART_INFO *info, unsigned char ip[])
{
	char *cmd = "AT+QIACT?\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	int rxnum = info->GetRxNum();
	char *pos = strstr((char *)rxbuff, "+QIACT: 1,1,1,");
	if (rxnum && pos) {
		int getip[4];
		int getnum = sscanf(pos, "+QIACT: 1,1,1,\"%d.%d.%d.%d\"", &getip[0], &getip[1],
				    &getip[2], &getip[3]);
		if (4 == getnum) {
			ip[0] = getip[0];
			ip[1] = getip[1];
			ip[2] = getip[2];
			ip[3] = getip[3];
			return 0;
		}
	}
	return -1;
}

int setQIACT(UART_INFO *info)
{
	char *cmd = "AT+QIACT=1\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	for (int i = 0; i < 10; i++) {
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, DEFAUTL_ACK_TIM);

		int rxnum = info->GetRxNum();
		char *pos = strstr((char *)rxbuff, "OK");
		if (rxnum && pos) {
			return 0;
		}

		pos = strstr((char *)rxbuff, "FAIL");
		if (rxnum && pos) {
			return -1;
		}
	}
	return -1;
}

int Connect4G(UART_INFO *pec200, unsigned char ips[])
{
	if (10 > getCSQ(pec200)) {
		return -1;
	}
	if (0 != getCPIN(pec200)) {
		return -2;
	}

	if (0 != getCREG(pec200)) {
		return -3;
	}
	if (0 != getCGATT(pec200)) {
		return -4;
	}
	if (0 != getQICSGP(pec200)) {
		return -5;
	}
	for (int i = 0; i < 5; i++) {
		//查看连接
		if (0 == getQIACT(pec200, ips)) {
			return 0;
		}
		//设置连接
		setQIACT(pec200);
	}
	return -8;
}

unsigned char ips[4];
void ec200_main(void *argument)
{
	UART_INFO *pec200 = get_myuart(1);

	while (1) {
		if (0 == Connect4G(pec200, ips)) {
		}
		osDelay(1000);
	}
}
