
#include "DMT.h"
#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "cmsis_os2.h"
#include "sbwt.h"

unsigned int crc_make(unsigned char *ptr, int len, unsigned int firstcrc)
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

int dmt_unpack_buff(DMT_UART_DATA_STRUCT *p, unsigned char *inbuff, int len, int using_crc)
{
	if (!p) {
		return -1;
	}

	for (int i = 0; i < len; i++) {
		unsigned char *testpos = &inbuff[i];
		int leftnum = len - i;
		//check head
		if (testpos[0] == 0x5a && testpos[1] == 0xa5) {
			//check command
			if ((testpos[3] == 0x82) || (testpos[3] == 0x83)) {
				//check len
				if (testpos[2] + 3 >= leftnum) {
					if (using_crc) {
						//check crc
						uint16_t mkcrc = crc_make(&testpos[3],
									  testpos[2] - 2, 0xffff);
						uint16_t getcrc = testpos[testpos[2] + 1] |
								  testpos[testpos[2] + 2] << 8;
						if (mkcrc == getcrc) {
							//p->dmt_sta = testpos[3];
							p->start_addr =
								(testpos[4] << 8) | testpos[5];
							p->dat = &testpos[6];
							p->datlen = testpos[2] - 3 - 2;
						}
					} else {
						//p->dmt_sta = testpos[3];
						p->start_addr = (testpos[4] << 8) | testpos[5];
						p->dat = &testpos[6];
						p->datlen = testpos[2] - 3;
					}

					return 0;
				}
			}
		}
	}
	return -2;
}

struct unpack_rd {
	int readlen;
	int addr;
	void *cpdatapos;
};

int unpackdata_rd(void *p, DMT_UART_DATA_STRUCT *pdat)
{
	if (!p || !pdat) {
		return -1;
	}

	unpack_rd *purd = (unpack_rd *)p;

	if (purd->addr == pdat->start_addr && purd->readlen == pdat->datlen - 1) {
		if (purd->cpdatapos) {
			memcpy(purd->cpdatapos, (unsigned char *)pdat->dat + 1, purd->readlen);
		}
		return 0;
	}
	return -2;
}

#if DMT_HAVE_WRITE_RESPONSE > 0
static int check_write_ok(void *p, unsigned char *buff, int len)
{
	const unsigned char right_char[] = { 0x5a, 0xa5, 0x03, 0x82, 0x4f, 0x4b };
	if (0 == memcmp(right_char, buff, sizeof(right_char))) {
		return 1;
	}
	return 0;
}
#endif

DMT::DMT_status DMT::write_data(int addr, void *buff, int &len)
{
	if (uartinfo) {
		if (len > 255 || len <= 0)
			return DMT_len_error;

		uartbufftx[0] = 0x5a;
		uartbufftx[1] = 0xa5;

		uartbufftx[2] = 3 + len;

		uartbufftx[3] = 0x82;

		uartbufftx[4] = addr >> 8;
		uartbufftx[5] = addr;

		memcpy(uartbufftx + 6, buff, len);

		if (using_crc) {
			uint16_t crc = crc_make(&uartbufftx[3], 3 + len, 0xffff);
			memcpy(uartbufftx + 6 + len, &crc, 2);
			uartbufftx[2] += 2;
			uartinfo->send(uartbufftx, 6 + len + 2);
		} else {
			uartinfo->send(uartbufftx, 6 + len);
		}

#if DMT_HAVE_WRITE_RESPONSE > 0
		if (0 == (check_write_ok, nullptr)) {
			return DMT_OK;
		}
#else
		uartinfo->wait_send_end();
		return DMT_OK;
#endif
	}
	return DMT_uart_not_init;
}

DMT::DMT_status DMT::read_data(int addr, void *buff, int &len)
{
	if (uartinfo) {
		if (len > 255 || len <= 0)
			return DMT_len_error;
		uartbufftx[0] = 0x5a;
		uartbufftx[1] = 0xa5;

		uartbufftx[2] = 4;

		uartbufftx[3] = 0x83;

		uartbufftx[4] = addr >> 8;
		uartbufftx[5] = addr;
		//写入读取字长度=len/2
		uartbufftx[6] = len / 2;

		if (using_crc) {
			uint16_t crc = crc_make(&uartbufftx[3], 3 + len, 0xffff);
			memcpy(uartbufftx + 7, &crc, 2);
			uartbufftx[2] += 2;
			uartinfo->send(uartbufftx, 7 + 2);
		} else {
			uartinfo->send(uartbufftx, 7);
		}

		unpack_rd urd;
		urd.addr = addr;
		urd.readlen = len;
		urd.cpdatapos = buff;

		if (0 == set_pro(unpackdata_rd, &urd)) {
			return DMT_OK;
		}
	}
	return DMT_uart_not_init;
}
