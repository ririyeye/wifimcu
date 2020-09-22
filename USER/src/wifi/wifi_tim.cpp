#include "stm32f10x.h"
#include "wifi/wifi_par.h"

extern "C" void TIM4_IRQHandler(void)
{
	auto *ESP8266_para = getWIFI_PAR();
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
		if (++ESP8266_para->milsec >= 1000) {
			ESP8266_para->milsec = 0;
			if (++ESP8266_para->sec >= 60) {
				ESP8266_para->sec = 0;
				if (++ESP8266_para->minute >= 60) {
					ESP8266_para->minute = 0;
					if (++ESP8266_para->hour >= 24) {
						ESP8266_para->hour = 0;
					}
				}
			}
		}
	}
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}


void timupdate_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
	uint16_t PrescalerValue = 0;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 36000000) - 1;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;//装载预分频器
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//增计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //允许定时器6更新中断
	/* TIM enable counter */
	TIM_Cmd(TIM4, ENABLE);
	/* Enable the TIM1 Capture Compare Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
