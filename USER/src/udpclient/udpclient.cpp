#include "udpclient/udpclient.h"
#include "string.h"
#include "cmsis_os2.h"
UDPFD udpfdgrp[2];

void udpinit(void)
{
	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		p.fd = i;
		p.connectID = i + 1;
		p.nowstatus = udpclosed;
		p.cmdstatus = udpclosed;
		p.rxend = 0;
		p.txend = 0;
		p.rxMax = RXMAX;
		p.txMax = TXMAX;
	}
}

void udpfd_set(UDPFD * fd, int status)
{
	if (!fd) {
		return;
	}

	if (status == updconnected) {
		fd->nowstatus = updconnected;
		fd->cmdstatus = updconnected;
	} else {
		fd->nowstatus = udpclosed;
		fd->cmdstatus = udpclosed;
	}
	fd->rxend = 0;
	fd->txend = 0;
	fd->rxMax = RXMAX;
	fd->txMax = TXMAX;
}




int udpopen_block(const char *addr, int port)
{
	if (!addr || port < 0) {
		return udp_par_error;
	}

	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		if (p.nowstatus == udpclosed) {
			strncpy(p.servername, addr, SERVERLEN);
			p.remoteport = port;
			p.cmdstatus = updconnected;
			for (int i = 0; i < 1000; i++) {
				if (p.nowstatus == updconnected)
					return p.fd;
				osDelay(1);
			}
		}
	}
	return udpnofd;
}

int udpclose_block(int fd)
{
	if (fd < 0) {
		return udp_par_error;
	}

	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		if (p.nowstatus == udpclosed) {
			p.cmdstatus = udpclosed;
			return 0;
		} else {
			p.cmdstatus = udpclosed;
			for (int i = 0; i < 1000; i++) {
				if (p.nowstatus == udpclosed)
					return 0;
				osDelay(1);
			}
			return udpoperator_failed;
		}
	}
	return udpnofd;
}

int udpread(int fd, unsigned char *dat, int maxlen)
{
	if (fd < 0 || !dat || maxlen <= 0) {
		return udp_par_error;
	}

	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		if (p.fd == fd) {
			if (p.nowstatus == updconnected) {
				if (p.rxend) {
					int retnum = 0;
					__disable_irq();
					if (maxlen >= p.rxend) {
						memcpy(dat, p.rxbuf, p.rxend);
						retnum = p.rxend;
						p.rxend = 0;
					} else {
						memcmp(dat, p.rxbuf, maxlen);
						memmove(p.rxbuf, &p.rxbuf[maxlen],
							p.rxend - maxlen);
						retnum = maxlen;
						p.rxend = p.rxend - maxlen;
					}
					__enable_irq();
					return retnum;
				}
			} else {
				return udpclosed;
			}
		}
	}
	return udpnofd;
}

int updwrite(int fd, unsigned char *dat, int len)
{
	if (fd < 0 || !dat || len <= 0) {
		return udp_par_error;
	}

	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		if (p.nowstatus == updconnected) {
			__disable_irq();
			int cplen = 0;
			if (p.txend + len > p.txMax)
				cplen = p.txMax - p.txend;
			else
				cplen = len;

			memcpy(&p.txbuf[p.txend], dat, cplen);
			p.txend += cplen;
			__enable_irq();
			return cplen;
		}
	}
	return udpnofd;
}

UDPFD *udpout(void)
{
	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		if (p.nowstatus == updconnected && p.txend > 0) {
			return &p;
		}
	}
	return nullptr;
}

void UDPFDitr_init(UDPFDitr *itr)
{
	if (itr)
		*itr = 0;
}

int UDPFDitr_getNext(UDPFDitr *itr)
{
	if (itr)
		if ((*itr >= 0) && (*itr < sizeof(udpfdgrp) / sizeof(UDPFD) - 1)) {
			*itr = *itr + 1;
			return 0;
		}

	return -1;
}

UDPFD *UDPFD_Get(UDPFDitr *itr)
{
	if (itr && (*itr >= 0) && (*itr < sizeof(udpfdgrp) / sizeof(UDPFD))) {
		return &udpfdgrp[*itr];
	}
	return nullptr;
}
