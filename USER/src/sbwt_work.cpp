#include "cmsis_os2.h"
#include "sbwt.h"
#include <stdint.h>
#include "DMT.h"
#include "ebyte_lora.h"
void sbwt_main(void *argument);

static uint64_t thread1_stk_1[64];

void init_sbwt()
{
	osThreadAttr_t thread1_attr;
	thread1_attr.stack_mem = &thread1_stk_1[0];
	thread1_attr.stack_size = sizeof(thread1_stk_1);
	osThreadId_t ost = osThreadNew(&sbwt_main, NULL, &thread1_attr);
}
#define SBWT_485_MAX_INDEX 50
SBWT_INFO sbinfo[SBWT_485_MAX_INDEX];

SBWT_INFO *GetSbwt_485(int index)
{
	for (auto &p : sbinfo) {
		if (p.SerialNum == index) {
			return &p;
		}
	}
	return nullptr;
}

void dec_sbwt_group_liveTim(SBWT_INFO *grp, int maxindex, int decnum)
{
	for (int i = 0; i < maxindex; i++) {
		if (grp[i].liveTime > 0) {
			grp[i].liveTime -= decnum;
		}
	}
}

SBWT_INFO *GetSbwt_all(int index)
{
	SBWT_INFO *slora = GetSbwt_lora(index);

	if (slora && slora->getAlive()) {
		return slora;
	}

	SBWT_INFO *s485 = GetSbwt_485(index);

	if (s485 && s485->getAlive()) {
		return s485;
	}

	if (s485) {
		return s485;
	}

	return nullptr;
}

static unsigned char txbuff[256];
static unsigned char rxbuff[256];

static SBWT_BUFF_STRUCT buff485 = { txbuff, rxbuff, 50, 5 };

static SBWT_BUFF_STRUCT *get485uartbuff()
{
	return &buff485;
}

void sbwt_main(void *argument)
{
	auto p3 = get_myuart(3);

	for (int i = 0; i < SBWT_485_MAX_INDEX; i++) {
		sbinfo[i].init(p3, i + 1, &buff485);
	}

	while (true) {
		//485特殊通讯
		dmt_ctl(p3, get485uartbuff(), spwork_uart_485);
		dec_sbwt_group_liveTim(sbinfo, SBWT_485_MAX_INDEX, 200);
		for (auto &p : sbinfo) {
			p.initTestSbwt();
		}
	}
}
