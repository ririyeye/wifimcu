#include "cmsis_os2.h"
#include "myuart.h"
#include "DMT.h"
#include "sbwt.h"
#include "string.h"
#include "stdio.h"
#include "stm32f10x.h"
DMT dmt1;
void disp_main(void *argument);

static uint64_t thread1_stk_1[64];
void dmt_ctl(UART_INFO *spuart, SBWT_BUFF_STRUCT *pbuf, unsigned int index)
{
	dmt1.SP_Work(spuart, pbuf, index);
}

void init_display()
{
	osThreadAttr_t thread1_attr;
	thread1_attr.stack_mem = &thread1_stk_1[0];
	thread1_attr.stack_size = sizeof(thread1_stk_1);
	osThreadId_t ost = osThreadNew(&disp_main, NULL, &thread1_attr);
}

char buff[40];

SBWT_INFO *GetSbwt(int index);

void show_data(DMT &dmt1, SBWT_INFO *info)
{
	int deg = info->GetData(1) * 360 / 16384;

	int absdeg = deg % 360;
	int rol = deg / 360;
	//圈数
	dmt1.write_int(0x2004, rol);
	//角度
	dmt1.write_int(0x2008, absdeg);
	//软件号
	dmt1.write_int(0x200c, info->s_variant);
	//软件版本
	dmt1.write_int(0x2010, info->s_version[0] * 100 + info->s_version[1]);
	dmt1.write_int(0x2014, info->h_version[0] * 10 + info->h_version[1]);
	//软件id
	dmt1.write_int(0x2018, info->mlx_soft_ver);
	//硬件id
	dmt1.write_int(0x201C, info->mlx_hard_ver);
	//ID
	int len = sprintf(buff, "%02x %02x %02x %02x %02x %02x", //
			  info->mlx_id[0], info->mlx_id[1], //
			  info->mlx_id[2], info->mlx_id[3], //
			  info->mlx_id[4], info->mlx_id[5]); //

	dmt1.write_data(0x2020, buff, len);

	int snddeg = absdeg < 0 ? 360 + absdeg : absdeg;
	snddeg *= 2;
	uint16_t tmp = __REV(snddeg) >> 16;
	len = 2;
	dmt1.write_data(0x2040, &tmp, len);
	//序列号
	dmt1.write_int(0x2080, info->SerialNum);
}

void dispIndex(int index, SBWT_INFO *info)
{
	int baseAddr = 0x1000 + 4 * index;

	if (info) {
		int deg = info->GetData(1) * 360 / 16384;
#if 0
		int absdeg = deg % 360;
		int rol = deg / 360;

		dmt1.write_int(0x1400, rol);
		dmt1.write_int(0x1404, absdeg);
#endif
		dmt1.write_int(baseAddr, deg);
	} else {
		dmt1.write_int(baseAddr, 0);
	}
}

void disp_main(void *argument)
{
	auto p3 = get_myuart(2);

	dmt1.init(p3);

	while (true) {
		int index = dmt1.getshowIndex();

		if (index > 0) {
			//设置界面
			SBWT_INFO *info = GetSbwt_all(index);

			if (info) {
				show_data(dmt1, info);
			} else {
				SBWT_INFO tmpinfo;
				show_data(dmt1, &tmpinfo);
			}
		} else {
			//数据界面
			int basenum = 1 + dmt1.get_page_num() * PER_PAGE_NUM;

			for (int i = 0; i < PER_PAGE_NUM; i++) {
				SBWT_INFO *info = GetSbwt_all(basenum + i);
				dispIndex(i, info);
			}
		}
		osDelay(10);
	}
}
