#include "udpclient/udpclient.h"

static UDPFD udpfdgrp[2];

void udpinit(void)
{
	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		p.fd = i;
		p.connectID = -1;
		p.nowstatus = udpclosed;
		p.cmdstatus = udpclosed;
		p.rxinit = 0;
		p.rxend = 0;
		p.txinit = 0;
		p.txend = 0;
	}
}

UDPFD *udpout(void)
{
	int sz = sizeof(udpfdgrp) / sizeof(UDPFD);
	for (int i = 0; i < sz; i++) {
		auto &p = udpfdgrp[i];
		if (p.txend > 0) {
			return &p;
		}
	}
	return nullptr;
}
