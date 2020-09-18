#include "jd/jdframe.h"
#include "myuart.h"
#include "cmsis_os2.h"
#include "myuart.h"
#include "stdio.h"
#include "stm32f10x.h"
#include <string.h>
#include "cc3200/cc3200.h"
#include "wifi/wifi_par.h"

static uint64_t thread_main_stk_1[64];

void JD_main(void *argument);

void init_JD(void)
{
	static osThreadAttr_t thread1_attr;
	thread1_attr.stack_mem = &thread_main_stk_1[0];
	thread1_attr.stack_size = sizeof(thread_main_stk_1);
	osThreadId_t ost = osThreadNew(&JD_main, NULL, &thread1_attr);
}

static unsigned char jd_txbuff[1024];
static unsigned char jd_rxbuff[1024];

static unsigned int crc_make(unsigned char *ptr, int len, unsigned int firstcrc)
{
	unsigned int crc = firstcrc;
	unsigned char i;
	while (len != 0) {
		if (len < 0)
			len = 0;
		crc ^= *ptr;
		for (i = 0; i < 8; i++) {
			if ((crc & 0x0001) == 0)
				crc = crc >> 1;
			else {
				crc = crc >> 1;
				crc ^= 0xa001;
			}
		}
		len -= 1;
		ptr++;
	}
	return crc;
}
static unsigned char crc_check(unsigned int a, unsigned char *Buff, unsigned int firstcrc,
			       unsigned char *match_byte)
{
	unsigned int crc0, crc1;

	if (match_byte == NULL) //使用数据末尾作为校验
	{
		if (a > 2) {
			crc0 = crc_make(Buff, (a - 2), firstcrc);
			crc1 = (Buff[a - 1] << 8) + Buff[a - 2];
		} else
			return 0;
	} else { //使用数据头的校验值作为校验
		crc0 = crc_make(Buff, a, firstcrc);
		crc1 = (match_byte[1] << 8) + match_byte[0];
	}
	if (crc0 == crc1)
		return 1;
	else
		return 0;
}

void mkSndBuf(unsigned char JDSndBuffer[], FRAME_DATA *rec, int downflg)
{
	static unsigned char thissndseq;

	const auto *pesp = getWIFI_PAR();
	JDSndBuffer[0] = 0xaa;
	JDSndBuffer[1] = 0xaa;
	JDSndBuffer[2] = rec->code;
	JDSndBuffer[3] = thissndseq++;

	if (downflg) {
		memcpy(&JDSndBuffer[4], pesp->thisdev, 4);
		memcpy(&JDSndBuffer[8], getaimdev(), 4);
	} else {
		memcpy(&JDSndBuffer[4], getaimdev(), 4);
		memcpy(&JDSndBuffer[8], pesp->thisdev, 4);
	}

	JDSndBuffer[12] = rec->frame_index;
	JDSndBuffer[13] = rec->frame_index >> 8;

	JDSndBuffer[14] = rec->len;
	JDSndBuffer[15] = rec->len >> 8;

	memcpy(&JDSndBuffer[16], rec->data, rec->len - MIN_PACK_SZ);

	int crc = crc_make(JDSndBuffer, rec->len - 2, 0xffff);

	JDSndBuffer[rec->len - 2] = crc;
	JDSndBuffer[rec->len - 1] = crc >> 8;
}

void mk_REC_DATA(unsigned char *data, int len, UART_INFO *info, FRAME_DATA *rec)
{
	if (!rec) {
		return;
	}

	rec->code = data[2];
	rec->seq = data[3];
	memcpy(rec->dst_dev, data + 4, 4);
	memcpy(rec->src_dev, data + 8, 4);

	rec->frame_index = data[12] | data[13] << 8;
	rec->len = data[14] | data[15] << 8;
	rec->data = data + 16;

	rec->crc = data[len - 2] | data[len - 1] << 8;

	rec->devInfo = info;

	rec->framedestroy = 0;
}

void sendREC(FRAME_DATA *rec)
{
	UART_INFO *psta = (UART_INFO *)rec->devInfo;

	if (!psta) {
		return;
	}

	mkSndBuf(jd_txbuff, rec, 1);

	if (rec->framedestroy) {
		rec->framedestroy(rec, rec->destorydat);
	}

	psta->wait_send_end();
	psta->send(jd_txbuff, rec->len);
	psta->wait_send_end();
}

int JD_Communication_data(unsigned char *rxbuf, int num, PROTOCOL_DAT *ret)
{
	for (int i = 0; i < num; i++) {
		int remainLen = num - i;
		if ((rxbuf[i] == 0XAA) && (rxbuf[i + 1] == 0XAA)) {
			if (remainLen >= MIN_PACK_SZ) {
				int recpackLen = rxbuf[i + 14] | (rxbuf[i + 15] << 8);
				if (recpackLen >= MIN_PACK_SZ && recpackLen <= MAX_PACK_SZ &&
				    remainLen >= recpackLen) {
					if (crc_check(recpackLen, &(*(rxbuf + i)), 0XFFFF, NULL) ==
					    1) {
						ret->datestart = &rxbuf[i];
						ret->datelen = recpackLen;
						return 0;
					}
				}
			}
		}
	}
	return -1;
}

void JD_main(void *argument)
{
	UART_INFO *pjd = get_myuart(3);
	pjd->opts->setspeed(230400);
	while (true) {
		pjd->rece(jd_rxbuff, sizeof(jd_rxbuff));
		pjd->wait_rece(100, 3);

		if (pjd->GetRxNum()) {
			PROTOCOL_DAT ret;

			if (0 == JD_Communication_data(jd_rxbuff, pjd->GetRxNum(), &ret)) {
				FRAME_DATA rec;
				mk_REC_DATA(ret.datestart, ret.datelen, pjd, &rec);
				pro_REC_DATA(&rec);
			}
		}
	}
}
