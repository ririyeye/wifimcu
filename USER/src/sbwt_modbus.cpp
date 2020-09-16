
#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "cmsis_os2.h"
#include "myuart.h"
#include "mymodbus.h"
#include "sbwt.h"
void sbwt_modbus_main(void *argument);

static uint64_t thread1_stk_1[64];

void init_sbwt_modbus()
{
	osThreadAttr_t thread1_attr;
	thread1_attr.stack_mem = &thread1_stk_1[0];
	thread1_attr.stack_size = sizeof(thread1_stk_1);
	osThreadId_t ost = osThreadNew(&sbwt_modbus_main, NULL, &thread1_attr);
}

struct sbwt_modbus_reactorf : public ModBus_rector {
	virtual int code() final
	{
		return 0x03;
	}
	virtual int prodata(unsigned char *indata, int len, unsigned char *outbuff,
			    unsigned int outbufflen) final
	{
		if (len == 4) {
			unsigned int initaddr = indata[0] | indata[1] << 8;
			unsigned int acquire_num = indata[2] | indata[3] << 8;

			if (initaddr + acquire_num < 250 && acquire_num < 50) {
				outbuff[0] = acquire_num * 4;
				for (int index = 0; index < acquire_num; index++) {
					unsigned char *datpos = &outbuff[1 + index * 4];
					SBWT_INFO *info = GetSbwt_all(index + 1);
					if (info && info->getAlive()) {
						*(unsigned int *)datpos = __REV(info->GetData(1));
					} else {
						*(int *)datpos = -1;
					}
				}
				return 1 + acquire_num * 4;
			}
		}
		return -1;
	}
};

sbwt_modbus_reactorf smr;
void sbwt_modbus_main(void *argument)
{
	auto p5 = get_myuart(5);
	ModBus_RTU rt;
	rt.init(1, p5);
	rt.setRectorGroup(&smr, 1);
	rt.poll_main();
}
