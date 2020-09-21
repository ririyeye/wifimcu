
#include "myuart.h"
#include "stm32f10x.h"
#include "cmsis_os2.h"
#include "stm32f10x_usart.h"

struct STM_UART_INFO : public UART_INFO {
	STM_UART_INFO(USART_TypeDef &puart) : huart(puart)
	{
	}

	STM_UART_INFO(USART_TypeDef &puart, UART_OPTS *inopts) : huart(puart)
	{
		opts = inopts;
	}

	USART_TypeDef &huart;

	unsigned char *txbuff = nullptr;
	int txpoint = 0;
	int txmax = 0;
	int txenable = 0;
	int rxenable = 0;

	osThreadId_t Thread_rcv = nullptr;
	osThreadId_t Thread_snd_end = nullptr;
#define WAIT_RX (1 << 0)
#define WAIT_TX_END (1 << 1)
	virtual int send(const void *buff, unsigned int num) final
	{
		if (buff && (num > 0)) {
			txenable = 1;
			txbuff = (unsigned char *)buff;
			txpoint = 0;
			txmax = num;

			USART_ITConfig(&huart, USART_IT_TXE, ENABLE);

			return num;
		}
		return -1;
	}

	unsigned char *rxbuff = nullptr;
	int rxpoint = 0;
	int rxmax = 0;

	virtual int rece(void *buff, unsigned int num) final
	{
		if (buff && (num > 0)) {
			rxenable = 1;
			rxbuff = (unsigned char *)buff;
			rxpoint = 0;
			rxmax = num;

			USART_ITConfig(&huart, USART_IT_RXNE, ENABLE);

			return num;
		}
		return -1;
	}

	virtual int wait_rece(unsigned int headtick, unsigned int tailtick) final
	{
		if (!rxenable)
			return -1;

		Thread_rcv = osThreadGetId();

		uint32_t ret = osThreadFlagsWait(WAIT_RX, 0, headtick);

		if (ret == osFlagsErrorTimeout) {
			return -2;
		}

		if (ret > 0xffff0000) {
			ret = ret > 0 ? -ret : ret;
		} else if (ret == WAIT_RX) {
			do {
				ret = osThreadFlagsWait(WAIT_RX, 0, tailtick);
			} while (ret == WAIT_RX); //等待接受完成
			ret = 0;
		}
		stopRX();
		return ret;
	}

	virtual int wait_send_end()
	{
		Thread_snd_end = osThreadGetId();
		while (0 != checkTXCPL()) {
			osThreadFlagsWait(WAIT_TX_END, 0, 1);
		}
		Thread_snd_end = nullptr;
		return 0;
	}

	virtual int checkTXCPL() final
	{
		//tx stopped == ret ok
		if (txenable == 0) {
			return 0;
		}
		return 1;
	}

	virtual int checkRXCPL() final
	{
		//rx stopped == ret ok
		if (rxenable == 0) {
			return 0;
		}
		return 1;
	}

	virtual void stopTX() final
	{
		txenable = 0;
	}

	virtual void stopRX() final
	{
		rxenable = 0;
	}

	virtual unsigned int GetRxNum() final
	{
		return rxpoint;
	}

	virtual unsigned int GetTxNum() final
	{
		return txpoint;
	}

	void usart_handle()
	{
		if (USART_GetFlagStatus(&huart, USART_FLAG_ORE) != RESET) //清除溢出错误
		{
			USART_ClearFlag(&huart, USART_FLAG_ORE);
			int data = USART_ReceiveData(&huart);//读取接收到的数据
			return;
		}
		
		if (USART_GetITStatus(&huart, USART_IT_RXNE) != RESET) {
			USART_ClearITPendingBit(&huart, USART_IT_RXNE);
			int data = USART_ReceiveData(&huart);//读取接收到的数据

			if (Thread_rcv) {
				osThreadFlagsSet(Thread_rcv, WAIT_RX);
			}

			if ((rxpoint < rxmax) && (rxbuff) && rxenable) {
				rxbuff[rxpoint++] = data;
				if (phandl)
					phandl->trig_rx();
			} else {
				rxenable = 0;
				USART_ITConfig(&huart, USART_IT_RXNE, ENABLE);
				if (phandl)
					phandl->trig_rx_cpl();
			}
			return;
		}

		if (USART_GetITStatus(&huart, USART_IT_TXE) != RESET) {
			if ((txpoint < txmax) && (txbuff) && txenable) {
				USART_SendData(&huart, txbuff[txpoint++]);
				if (phandl)
					phandl->trig_tx();
			} else {
				txenable = 0;
				USART_ITConfig(&huart, USART_IT_TXE, DISABLE);
				if (phandl)
					phandl->trig_tx_cpl();
				if (Thread_snd_end) {
					osThreadFlagsSet(Thread_snd_end, WAIT_TX_END);
				}
			}
			return;
		}
	}
};

/*
* link to hardware
*/
int uart1_init(unsigned int bound);
int uart2_init(unsigned int bound);
int uart3_init(unsigned int bound);
int uart4_init(unsigned int bound);
int uart5_init(unsigned int bound);

#if MAX_UART_INDEX >= 5
STM_UART_INFO stmuart5(*UART5);
#endif

#if MAX_UART_INDEX >= 4
STM_UART_INFO stmuart4(*UART4);
#endif

static struct UART_OPTS u3opts = {
	.setspeed = uart3_init,
};
STM_UART_INFO stmuart3(*USART3, &u3opts);

STM_UART_INFO stmuart2(*USART2);
static struct UART_OPTS u1opts = {
	.setspeed = uart1_init,
};

STM_UART_INFO stmuart1(*USART1, &u1opts);

#if MAX_UART_INDEX >= 5
extern "C" void UART5_IRQHandler(void)
{
	stmuart5.usart_handle();
}
#endif
#if MAX_UART_INDEX >= 4
extern "C" void UART4_IRQHandler(void)
{
	stmuart4.usart_handle();
}
#endif
extern "C" void USART3_IRQHandler(void)
{
	stmuart3.usart_handle();
}

extern "C" void USART2_IRQHandler(void)
{
	stmuart2.usart_handle();
}

extern "C" void USART1_IRQHandler(void)
{
	stmuart1.usart_handle();
}

struct uart_connect {
	int index;
	STM_UART_INFO *info;
};

static uart_connect cons[] = {
	{ 1, &stmuart1 }, //绑定串口1
	{ 2, &stmuart2 }, //绑定串口2
	{ 3, &stmuart3 }, //绑定串口3
#if MAX_UART_INDEX >= 4
	{ 4, &stmuart4 }, //绑定串口4
#endif
#if MAX_UART_INDEX >= 5
	{ 5, &stmuart5 }, //绑定串口5
#endif
};

UART_INFO *get_myuart(int index)
{
	for (auto &con : cons) {
		if (index == con.index) {
			return con.info;
		}
	}
	return nullptr;
}
