#include "ebyte_lora.h"
#include "cmsis_os2.h"
#include "myuart.h"
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include "sbwt.h"
#include <DMT.h>

int ebyte_operator::init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //GPIOC时钟 重映射时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	//md0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	//GPIO_Init(GPIOC, &GPIO_InitStructure);
	//md1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	return 0;
}

void ebyte_operator::Lora_MD0_write(int val)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_0, val ? Bit_SET : Bit_RESET);
}

void ebyte_operator::Lora_MD1_write(int val)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_2, val ? Bit_SET : Bit_RESET);
}

static uint64_t thread_main_stk_1[64];

void ebyte_lora_main(void *argument);

void init_ebyte_lora()
{
	static osThreadAttr_t thread1_attr;
	thread1_attr.stack_mem = &thread_main_stk_1[0];
	thread1_attr.stack_size = sizeof(thread_main_stk_1);
	osThreadId_t ost = osThreadNew(&ebyte_lora_main, NULL, &thread1_attr);
}

static unsigned char txbuff[256];
static unsigned char rxbuff[256];

static SBWT_BUFF_STRUCT lorabuff = { txbuff, rxbuff, 2000, 20 };

static SBWT_BUFF_STRUCT *getlorauartbuff()
{
	return &lorabuff;
}

#define MAX_LORA_NODE 10
SBWT_INFO sbinfo_lora[MAX_LORA_NODE];

SBWT_INFO *GetSbwt_lora(int index)
{
	for (auto &p : sbinfo_lora) {
		if (p.SerialNum == index) {
			return &p;
		}
	}
	return nullptr;
}
extern DMT dmt1;
#define ADAPTIVE_LORA_CONNECT 0
void ebyte_lora_main(void *argument)
{
	ebyte_operator eb;
	eb.init();
	eb.Lora_MD0_write(0);
	eb.Lora_MD1_write(0);

	auto p4 = get_myuart(4);

	for (int i = 0; i < MAX_LORA_NODE; i++) {
		sbinfo_lora[i].init(p4, i + 1, getlorauartbuff());
	}

	while (true) {
		dec_sbwt_group_liveTim(sbinfo_lora, MAX_LORA_NODE, 100);
		for (auto &loranode : sbinfo_lora) {
			//lora特殊通信
			dmt_ctl(p4, getlorauartbuff(), spwork_uart_lora);
#if ADAPTIVE_LORA_CONNECT > 0
			int spindex = dmt1.getshowIndex();
			if (spindex > 0 && GetSbwt_lora(spindex)) {
				GetSbwt_lora(spindex)->initTestSbwt();
			} else {
				loranode.initTestSbwt();
			}
#else
			loranode.initTestSbwt();
#endif
		}
	}
}
