#include "mymodbus.h"
#include <string.h>

void myModBus::poll_main()
{
	if (pinfo) {
		pinfo->rece(rxbuff, 250);
		if (0 <= pinfo->wait_rece(50, 5)) {
			int rxnum = pinfo->GetRxNum();
			if (rxnum) {
				int ret = proRec(rxbuff, rxnum, txbuff, 250);
				if (0 < ret) {
					pinfo->send(txbuff, ret);
				}
			}
		}
	}
}

extern "C" unsigned short usMBCRC16(unsigned char *pucFrame, unsigned short usLen);

int ModBus_RTU::proRec(unsigned char *buff, unsigned int len, unsigned char *outbuff,
		       unsigned int outbufflen)
{
	if (buff[0] != addr && buff[0] != 0) {
		return -1;
	}

	if (usMBCRC16(buff, len) != 0) {
		return -2;
	}

	unsigned char code = buff[1];
	unsigned char *data = &buff[2];
	int datalen = len - 4;

	if (grp && grpsz) {
		for (int i = 0; i < grpsz; i++) {
			if (code == grp[i].code()) {
				int ret =
					grp[i].prodata(data, datalen, outbuff + 2, outbufflen - 2);
				if (ret > 0) {
					outbuff[0] = addr;
					outbuff[1] = code;
					//mkcrc
					uint16_t usCRC16 = usMBCRC16(outbuff, ret + 2);
					outbuff[ret + 2] = (unsigned char)(usCRC16 & 0xFF);
					outbuff[ret + 3] = (unsigned char)(usCRC16 & 0xFF);
					return ret + 4;
				}
			}
		}
	}
	return 0;
}

void ModBus_RTU::setRectorGroup(ModBus_rector *ingrp, int sz)
{
	if (ingrp && sz) {
		grp = ingrp;
		grpsz = sz;
	}
}
