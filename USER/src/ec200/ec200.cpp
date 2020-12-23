#include "ec200/ec200.h"
#include "cmsis_os2.h"
#include "myuart.h"
#include "stdio.h"
#include "stm32f10x.h"
#include <string.h>
#include "udpclient/udpclient.h"

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

int closeEcho(UART_INFO *info)
{
	char *cmd = "AT+QISDE=0\r\n";
	info->send(cmd, strlen(cmd));
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);
	return 0;
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
	closeEcho(pec200);
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

int getUDP_client_sock(UART_INFO *info, int sockID, const char *server, int remote_port,
		       int local_port)
{
	int snlen = snprintf((char *)txbuff, 128, "AT+QIOPEN=1,%d,\"UDP\",\"%s\",%d,%d,0\r\n",
			     sockID, server, remote_port, local_port);

	info->send(txbuff, snlen);

	for (int i = 0; i < 10; i++) {
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, DEFAUTL_ACK_TIM);

		int rxnum = info->GetRxNum();
		char *pos = strstr((char *)rxbuff, "+QIOPEN:");
		if (rxnum && pos) {
			int recport, sta;
			if (2 == sscanf(pos, "+QIOPEN:%d,%d", &recport, &sta)) {
				return 0;
			}
		}

		pos = strstr((char *)rxbuff, "FAIL");
		if (rxnum && pos) {
			return -1;
		}
	}
	return -1;
}

int ec200_udpsend(UART_INFO *info, int sockID, unsigned char *data, int len)
{
	int snlen = snprintf((char *)txbuff, 1024, "AT+QISEND=%d,%d\r\n", sockID, len);
	info->send(txbuff, snlen);
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	int rxnum = info->GetRxNum();
	char *pos = strstr((char *)rxbuff, ">");

	if (rxnum && pos) {
		info->send(data, len);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, DEFAUTL_ACK_TIM);
		rxnum = info->GetRxNum();
		pos = strstr((char *)rxbuff, "SEND OK");
		return len;
	}
	return -1;
}

int ec200_udprecv(UART_INFO *info, int sockID, unsigned char **data, int maxlen)
{
	int snlen = snprintf((char *)txbuff, 1024, "AT+QIRD=%d,%d\r\n", sockID, maxlen);
	info->send(txbuff, snlen);
	info->rece(rxbuff, CountOf(rxbuff));
	info->wait_rece(200, DEFAUTL_ACK_TIM);

	int rxnum = info->GetRxNum();
	char *pos = strstr((char *)rxbuff, "+QIRD:");

	if (rxnum && pos) {
		int getlen = -1;
		int sclen = sscanf(pos, "+QIRD: %d", &getlen);
		if ((1 == sclen) && (getlen > 0)) {
			char *datpos = strstr(pos, "\r\n");
			if (datpos && (datpos - pos < 11)) {
				if (0 == memcmp("\r\nOK\r\n", datpos + getlen + 4, 6)) {
					if (data)
						*data = (unsigned char *)&datpos[2];
					return getlen;
				}
			}
		}
	}
	return -1;
}

int ec200_udp_close(UART_INFO *info, int sockID)
{
	int snlen = snprintf((char *)txbuff, 1024, "AT+QICLOSE=%d\r\n", sockID);
	info->send(txbuff, snlen);

	for (int i = 0; i < 5; i++) {
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, DEFAUTL_ACK_TIM);
		int rxnum = info->GetRxNum();
		char *pos = strstr((char *)rxbuff, "OK");
		if (pos) {
			return 0;
		}
		pos = strstr((char *)rxbuff, "ERROR");
		if (pos) {
			return -1;
		}
	}
	return -1;
}
static unsigned char ips[4];

int ec200_udpsend_seq(UART_INFO *info)
{
	UDPFD *p = udpout();
	int ret = 0;
	while (p) {
		int sndlen = ec200_udpsend(info, p->connectID, p->txbuf, p->txend);
		if (0 < sndlen) {
			__disable_irq();
			if (p->txend <= sndlen) {
				p->txend = 0;
			} else {
				memmove(p->txbuf, &p->txbuf[sndlen], p->txend - sndlen);
				p->txend -= sndlen;
			}
			ret++;
			__enable_irq();
		}
		p = udpout();
	}
	return ret;
}

int ec200_poll_read(UART_INFO *info)
{
	UDPFDitr itr;
	UDPFDitr_init(&itr);
	UDPFD *fd = UDPFD_Get(&itr);
	int ret = 0;
	while (fd) {
		if (fd->nowstatus == updconnected) {
			unsigned char *rtdatapos;
			int len = ec200_udprecv(info, fd->connectID, &rtdatapos,
						fd->rxMax - fd->rxend);
			if (len > 0) {
				__disable_irq();
				memcpy(&fd->rxbuf[fd->rxend], rtdatapos, len);
				fd->rxend += len;
				ret++;
				__enable_irq();
			}
		}
		if (0 == UDPFDitr_getNext(&itr)) {
			fd = UDPFD_Get(&itr);
		} else {
			fd = nullptr;
		}
	}
	return ret;
}

int ec200_cmd_ctrl(UART_INFO *info)
{
	UDPFDitr itr;
	UDPFDitr_init(&itr);
	UDPFD *fd = UDPFD_Get(&itr);
	int ret = 0;
	while (fd) {
		if (fd->nowstatus != fd->cmdstatus) {
			if (fd->cmdstatus == updconnected) {
				int sta = getUDP_client_sock(info, fd->connectID, fd->servername,
							     fd->remoteport, 0);
				if (0 == sta) {
					udpfd_set(fd, updconnected);
					ret++;
				}
			} else {
				int sta = ec200_udp_close(info, fd->connectID);
				if (0 == sta) {
					udpfd_set(fd, udpclosed);
					ret++;
				}
			}
		}
		if (0 == UDPFDitr_getNext(&itr)) {
			fd = UDPFD_Get(&itr);
		} else {
			fd = nullptr;
		}
	}
	return ret;
}

void ec200_main(void *argument)
{
	UART_INFO *pec200 = get_myuart(1);

	while (1) {
		if (0 == Connect4G(pec200, ips)) {
			//TODO 没有判断ec200 通讯报错需要重连
			while (1) {
				int cn = ec200_cmd_ctrl(pec200);
				int sn = ec200_udpsend_seq(pec200);
				int rd = ec200_poll_read(pec200);
				if (sn <= 0 && rd <= 0 && cn <= 0) {
					osDelay(1);
				}
			}
		}
	}
}
