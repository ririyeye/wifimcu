#include "cmsis_os2.h"
#include "myuart.h"
#include "stdio.h"
#include "stm32f10x.h"
#include <string.h>
#include "cc3200/cc3200.h"
template <typename T, size_t N> inline size_t CountOf(T (&arr)[N])
{
	return N;
}


static uint64_t thread_main_stk_1[64];

void cc3200_main(void *argument);

void init_cc3200(void)
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

int enter_at_mode(UART_INFO *info, int *speedgrp, int speedcnt)
{
	int txnum = sprintf((char *)txbuff, "+++");

	for (int retrycnt = 0; retrycnt < 100; retrycnt++) {
		info->opts->setspeed(speedgrp[retrycnt % speedcnt]);

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

int get_ssid(UART_INFO *info, char *ssid, char *passwd)
{
	for (size_t i = 0; i < 10; i++) {
		char *cmd = "AT+STA=?\r\n";
		info->send(cmd, strlen(cmd));
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, 200);

		int cnt;
		int scnum = sscanf((char *)rxbuff, "SSID:%s TYPE:%d Password:%s\r\n", ssid, &cnt,
				   passwd);

		if (scnum > 0) {
			return 0;
		}
	}
	return -1;
}

int set_ssid(UART_INFO *info, char *ssid, char *passwd)
{
	int snlen = 0;
	if (passwd) {
		snlen = sprintf((char *)txbuff, "AT+STA=%s,2,%s\r\n", ssid, passwd);
	} else {
		snlen = sprintf((char *)txbuff, "AT+STA=%s,0\r\n", ssid);
	}

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "STA update OK")) {
			return 0;
		}
	}
	return -1;
}

int set_mod_sta(UART_INFO *info)
{
	for (size_t i = 0; i < 10; i++) {
		char *cmd = "AT+ROLE=STA\r\n";
		info->send(cmd, strlen(cmd));
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, 200);

		if (info->GetRxNum() && strstr((char *)rxbuff, "Set STA mode")) {
			return 0;
		}
	}
	return -1;
}

int set_fix_ip_port(UART_INFO *info, const char *staip, const char *mask, const char *gatewayip)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+STAIP=STATIC,%s,%s,%s,%s\r\n", staip, mask, gatewayip,
			"223.5.5.5");

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "STAIP update OK")) {
			return 0;
		}
	}
	return -1;
}

int set_server_ip_port(UART_INFO *info, const char *serverip, int port)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+SOCK=TCP,CLIENT,%s,%d,1234\r\n", serverip, port);

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Socket update OK")) {
			return 0;
		}
	}
	return -1;
}

int set_exit(UART_INFO *info)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+EXIT\r\n");

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Exited AT mode")) {
			return 0;
		}
	}
	return -1;
}

int set_PM(UART_INFO *info, int mod)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+PM=%d,240\r\n", mod);

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Power mode set OK")) {
			return 0;
		}
	}
	return -1;
}

int set_uart_speed(UART_INFO *info, int speed)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+UART=%d,8,0,1\r\n", speed);

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Uart update OK")) {
			return 0;
		}
	}
	return -1;
}

char *ip = "192.168.50.188";
char *serverip = "192.168.50.205";
int speeds[] = { 115200 * 1, 115200 * 10 };

void cc3200_main(void *argument)
{
	init_cc3200_gpio();

	UART_INFO *pwifi = get_myuart(1);

	enter_at_mode(pwifi, speeds, CountOf(speeds));

	set_mod_sta(pwifi);

	set_ssid(pwifi, "XINCHEN-2.4G", "1000000001");

	set_fix_ip_port(pwifi, ip, "255.255.255.0", "192.168.50.1");

	set_server_ip_port(pwifi, serverip, 8888);

	set_PM(pwifi, 0);

	set_uart_speed(pwifi, speeds[CountOf(speeds) - 1]);

	set_exit(pwifi);

	pwifi->opts->setspeed(speeds[CountOf(speeds) - 1]);

	while (true) {
	}
}
