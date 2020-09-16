#include "DMT.h"
#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "cmsis_os2.h"
#include "sbwt.h"

DMT::DMT_status DMT::write_int(int addr, int data)
{
	unsigned int tmp = __REV(data);
	int len = 4;
	return write_data(addr, &tmp, len);
}

DMT::DMT_status DMT::write_float(int addr, float data)
{
	unsigned int tmp = __REV(*(int *)&data);
	int len = 4;
	return write_data(addr, &tmp, len);
}

static uint64_t thread_rec_stk_1[64];

void disp_rec(void *arg)
{
	if (arg) {
		DMT *pdmt = (DMT *)arg;
		pdmt->rec_thread();
	}
}

void DMT::init(UART_INFO *inuart)
{
	if (inuart) {
		setuart(inuart);
	}

	osThreadAttr_t thread2_attr;
	thread2_attr.stack_mem = &thread_rec_stk_1[0];
	thread2_attr.stack_size = sizeof(thread_rec_stk_1);
	osThreadId_t ost2 = osThreadNew(&disp_rec, this, &thread2_attr);
}

void DMT::rec_thread()
{
	DMT_UART_DATA_STRUCT duds;
	while (true) {
		uartinfo->rece(uartbuffrx, 250);
		if (0 <= uartinfo->wait_rece(50, 5)) {
			int rdlen = uartinfo->GetRxNum();

			int ret = dmt_unpack_buff(&duds, uartbuffrx, rdlen, using_crc);

			if (ret == 0) {
				write_rec(&duds);
				default_rec(&duds);
			}
		}
	}
}

int DMT::default_rec(DMT_UART_DATA_STRUCT *pdat)
{
	//auto upload data len = 3 && wordlen = 2
	if (pdat && pdat->datlen == 3) {
		unsigned char *tmpbuf = (unsigned char *)pdat->dat;
		int16_t tmpdat = (tmpbuf[1] << 8) | tmpbuf[2];

		read_addr_num(pdat->start_addr, tmpdat);
	}
	return 0;
}

int setNewID(UART_INFO *spuart, SBWT_BUFF_STRUCT *pbuf, int serialnum)
{
	SBWT_INFO tmpinfo;
	tmpinfo.init(spuart, 0xff, pbuf);
	if (0 != tmpinfo.initSBWT()) {
		return -1;
	}

	tmpinfo.ChangeNum(serialnum);
	return 0;
}

int setZeroCalibration(UART_INFO *spuart, SBWT_BUFF_STRUCT *pbuf, int serialnum)
{
	SBWT_INFO tmpinfo;
	tmpinfo.init(spuart, serialnum, pbuf);
	if (0 != tmpinfo.initSBWT()) {
		return -1;
	}

	tmpinfo.SetZeroCalibration();
	return 0;
}

void DMT::SP_Work(UART_INFO *spuart, SBWT_BUFF_STRUCT *pbuf, unsigned int spwork_index)
{
	if (spwork_index > SPWORK_UART_MAX_TYPE_ID) {
		return;
	}

	//只有485才能修改地址,禁止用lora修改地址
	if (changeFlag[spwork_index] > 0 && spwork_index == spwork_uart_485) {
		setNewID(spuart, pbuf, changeFlag[spwork_index]);
		changeFlag[spwork_index] = 0;
	}

	if (correctFlag[spwork_index] > 0) {
		setZeroCalibration(spuart, pbuf, correctFlag[spwork_index]);
		correctFlag[spwork_index] = 0;
	}
}

void DMT::read_addr_num(int addr, int value)
{
	//进入设置
	if (addr == 0x2000) {
		//change to set
		if (value > 0 &&  pageindex < MAX_PAGE_NUM) {
			showindex = pageindex * PER_PAGE_NUM + value;
		} else {
			showindex = -1;
		}

		return;
	}
	//页修改
	if (addr == 0x2108) {
		pageindex = value - 0x10;
		return;
	}
	//校正设置
	if (addr == 0x2100) {
		correctFlag[spwork_uart_485] = showindex;
		correctFlag[spwork_uart_lora] = showindex;
		return;
	}
	//修改设置
	if (addr == 0x2104) {
		changeFlag[spwork_uart_485] = showindex;
#if 0
		//禁止用lora修改地址
		changeFlag[spwork_uart_lora] = showindex;
#endif
		return;
	}
}

int DMT::write_rec(DMT_UART_DATA_STRUCT *pdat)
{
	if (profun && pdat) {
		if (0 == (*profun)(proData, pdat)) {
			if (write_id) {
				osThreadFlagsSet(write_id, 1);
				return 1;
			}
		}
	}
	return 0;
}

int DMT::set_pro(protype fun, void *fundata)
{
	profun = &fun;
	proData = fundata;
	write_id = osThreadGetId();

	int ret = osThreadFlagsWait(1, 0, 100);

	if (ret < 0) {
		ret = -1;
	} else {
		ret = 0;
	}

	profun = nullptr;
	proData = nullptr;
	write_id = 0;

	return ret;
}
