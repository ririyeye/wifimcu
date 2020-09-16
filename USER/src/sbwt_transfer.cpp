#include "sbwt.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

unsigned int CRC16ppp(unsigned char *buff, unsigned int len)
{
	unsigned char bitloop;
	unsigned int crcsum;
	unsigned int tmp;

	crcsum = 0xffff;

	while (len != 0) {
		tmp = (*buff++ << 8);
		for (bitloop = 0; bitloop < 8; bitloop++) {
			if ((crcsum ^ tmp) & 0x8000)
				crcsum = (crcsum << 1) ^ 0x1021;
			else
				crcsum <<= 1;

			tmp <<= 1;
		}
		len--;
	}
	return crcsum;
}

int mkTpack(TPack *pack, unsigned char *outBuff)
{
	outBuff[0] = 0x10;
	outBuff[1] = 0x02;
	outBuff[2] = pack->addr;
	outBuff[3] = pack->code;
	outBuff[4] = 0;

	memcpy(outBuff + 5, pack->buff, pack->sz);

	uint16_t crc16 = CRC16ppp(outBuff + 2, 3 + pack->sz);

	outBuff[5 + pack->sz] = crc16 >> 8;
	outBuff[6 + pack->sz] = crc16;
	outBuff[7 + pack->sz] = 0x10;
	outBuff[8 + pack->sz] = 0x03;
	return 9 + pack->sz;
}

__inline int compByte(unsigned char *input, unsigned char *aim, unsigned int aimLen)
{
	for (int i = 0; i < aimLen; i++) {
		if (input[i] != aim[i]) {
			return -1;
		}
	}
	return 0;
}

int findbyte(unsigned char *input, unsigned int inputLen, unsigned char *aim, unsigned int aimLen)
{
	for (int i = 0; i <= inputLen - aimLen; i++) {
		if (0 == compByte(input + i, aim, aimLen)) {
			return i;
		}
	}
	return -1;
}

int unTpakc(TPack *pack, unsigned char *inbuff, unsigned int sz)
{
	if (!pack) {
		return -3;
	}

	//find head
	unsigned char headPattern[2] = { 0x10, 0x02 };
	int headPos = findbyte(inbuff, sz, headPattern, 2);
	if (headPos < 0) {
		return -1;
	}
	//find tail
	unsigned char tailPattern[2] = { 0x10, 0x03 };
	int tailPos = findbyte(inbuff, sz, tailPattern, 2);

	if (tailPos < 0) {
		return -1;
	}
	//check crc;
	int crclen = tailPos - headPos - 2 - 2;
	if (crclen < 0) {
		return -2;
	}
	uint16_t crc16 = CRC16ppp(inbuff + headPos + 2, crclen);

	unsigned char crchigh = crc16 >> 8;
	unsigned char crclow = crc16;

	if ((inbuff[tailPos - 2] != crchigh) || (inbuff[tailPos - 1] != crclow)) {
		return -3;
	}
	//decode pack
	pack->addr = inbuff[headPos + 2];
	pack->code = inbuff[headPos + 3];

	int datalen = tailPos - headPos;
	if (datalen <= 7) {
		pack->buff = nullptr;
		pack->sz = 0;
	} else {
		pack->buff = &inbuff[headPos + 5];
		pack->sz = datalen - 7;
	}
	return 0;
}

#if 0
int SBWT_INFO::scanfInit()
{
	for (auto p : sbdata) {
		if (!p.enable) {
			unsigned char askforinit[] = { 0x10 , 0x05 , (unsigned char)p.addr };
			pinfo->send(askforinit, 3);
			pinfo->rece(rxbuff, 250);
			if (0 <= pinfo->wait_rece(buff->rxdelay, buff->waitcpl)) {
				int rxnum = pinfo->GetRxNum();
				if (rxnum) {
					TPack getpack;
					if (0 == unTpakc(&getpack, rxbuff, rxnum)) {
						checkAppData(&getpack, *this);
					}
				}
			}
		}
	}
}

#endif

int SBWT_INFO::initSBWT()
{
	unsigned char softid[] = { 1, 0, 5 };
	TPack pack = { SerialNum, 40, softid, 3 };
	int packlen = mkTpack(&pack, buff->txbuff);
	pinfo->send(buff->txbuff, packlen);
	pinfo->rece(buff->rxbuff, 250);

	if (0 > pinfo->wait_rece(buff->rxdelay, buff->waitcpl)) {
		int rxnum = pinfo->GetRxNum();
		if (0 >= rxnum) {
			return -1;
		}
	}

	unTpakc(&pack, buff->rxbuff, pinfo->GetRxNum());
	unsigned char *rdbuf = pack.buff;
	s_variant = rdbuf[0];
	s_version[0] = rdbuf[1];
	s_version[1] = rdbuf[2];
	h_version[0] = rdbuf[3];
	h_version[1] = rdbuf[4];
	mlx_soft_ver = rdbuf[5];
	mlx_hard_ver = rdbuf[6];
	memcpy(mlx_id, &rdbuf[7], 6);

	unsigned char setbaseinit[] = { 0, 2, 0x8d, 0x07 };
	pack = { SerialNum, 62, setbaseinit, 4 };
	packlen = mkTpack(&pack, buff->txbuff);
	pinfo->send(buff->txbuff, packlen);
	pinfo->rece(buff->rxbuff, 250);
	pinfo->wait_rece(buff->rxdelay, buff->waitcpl);
	return 0;
}

int SBWT_INFO::ChangeNum(int newNum)
{
	unsigned char addrAndID[7] = { (unsigned char)newNum };
	memcpy(&addrAndID[1], mlx_id, 6);

	TPack pack = { SerialNum, 101, addrAndID, 7 };

	int packlen = mkTpack(&pack, buff->txbuff);
	pinfo->send(buff->txbuff, packlen);
	pinfo->rece(buff->rxbuff, 250);

	if (0 > pinfo->wait_rece(buff->rxdelay, buff->waitcpl)) {
		int rxnum = pinfo->GetRxNum();
		if (0 >= rxnum) {
			return -1;
		}
	}
	return 0;
}

int SBWT_INFO::SetZeroCalibration()
{
	unsigned char ChangeCode[3] = { 0x08, 0x3b, 0xef };

	TPack pack = { SerialNum, 0x48, ChangeCode, sizeof(ChangeCode) };

	int packlen = mkTpack(&pack, buff->txbuff);
	pinfo->send(buff->txbuff, packlen);
	pinfo->rece(buff->rxbuff, 250);

	if (0 > pinfo->wait_rece(buff->rxdelay, buff->waitcpl)) {
		int rxnum = pinfo->GetRxNum();
		if (0 >= rxnum) {
			return -1;
		}
	}
	return 0;
}

int SBWT_INFO::GetDataSBWT()
{
	TPack pack = { SerialNum, 0x30, nullptr, 0 };
	int packlen = mkTpack(&pack, buff->txbuff);
	pinfo->send(buff->txbuff, packlen);

	pinfo->rece(buff->rxbuff, 250);
	if (0 <= pinfo->wait_rece(buff->rxdelay, buff->waitcpl)) {
		int rxnum = pinfo->GetRxNum();
		if (rxnum) {
			TPack getpack;
			if (0 == unTpakc(&getpack, buff->rxbuff, rxnum)) {
				checkAppData(&getpack, *this);
			}
		}
	}
	return 0;
}

int SBWT_INFO::initTestSbwt()
{
	if (pinfo) {
		if (0 != initSBWT()) {
#if 0
			setAlive(0);
#endif
			return sta_not_init;
		} else {
			setAlive(1000);
		}

		if (0 <= GetDataSBWT()) {
		}
	}
	return sta_ok;
}
