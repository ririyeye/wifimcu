
#include "myuart.h"
#include "stm32f10x.h"
#include "cmsis_os2.h"
#include "stm32f10x_usart.h"

void uart5_init(unsigned int bound)
{
	//初始化IO 串口5
	//bound:波特率
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
			       ENABLE); //GPIOC时钟 重映射时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE); //使能USART5
	USART_DeInit(UART5); //复位串口1
		//UART5_TX   PC.12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PB10

	//UART5_RX	  PD.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOD, &GPIO_InitStructure); //初始化PC11

	//	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, DISABLE);//USART3 部分重映射
	//	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);//USART3 部分重映射
	//	AFIO->MAPR &= 0xFFFFFFCF;
	//Usart3 NVIC 配置

	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound; //一般设置为9600;
#if 0
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_Even;//偶校验
#else
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //偶校验
#endif
	USART_InitStructure.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式

	USART_Init(UART5, &USART_InitStructure); //初始化串口
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE); //开启中断

	USART_Cmd(UART5, ENABLE); //使能串口
}

void uart4_init(unsigned int bound)
{
	//初始化IO 串口4
	//bound:波特率
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //GPIOC时钟 重映射时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE); //使能USART3
	USART_DeInit(UART4); //复位串口1
		//UART4_TX   PC.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PB10

	//UART4_RX	  PC.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PC11

	//	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, DISABLE);//USART3 部分重映射
	//	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);//USART3 部分重映射
	//	AFIO->MAPR &= 0xFFFFFFCF;
	//Usart3 NVIC 配置

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound; //一般设置为9600;

#if 0
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_Even;//偶校验
#else
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //偶校验
#endif

	USART_InitStructure.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式

	USART_Init(UART4, &USART_InitStructure); //初始化串口
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); //开启中断

	USART_Cmd(UART4, ENABLE); //使能串口
}

void uart3_init(unsigned int bound)
{
	//初始化IO 串口3
	//bound:波特率
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,
			       ENABLE); //GPIOC时钟 重映射时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //使能USART3
	USART_DeInit(USART3); //复位串口1
		//USART3_TX   PB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10

	//USART3_RX	  PB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PC11

	//	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, DISABLE);//USART3 部分重映射
	//	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);//USART3 部分重映射
	//	AFIO->MAPR &= 0xFFFFFFCF;
	//Usart3 NVIC 配置

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound; //一般设置为9600;
#if 1
	USART_InitStructure.USART_WordLength = USART_WordLength_9b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_Even; //偶校验
#else
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //偶校验
#endif
	USART_InitStructure.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //开启中断

	USART_Cmd(USART3, ENABLE); //使能串口
}

void uart2_init(unsigned int bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //开启U2串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	USART_DeInit(USART2); //复位串口2

	/* JDBus TX-PA2 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //TX PA2 推挽复用输出模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* JDBus RX-PA3  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //RX PA3浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound; //设置为19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //开启中断

	USART_Cmd(USART2, ENABLE); //使能串口
}

void uart1_init(unsigned int bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //开启U1串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	USART_DeInit(USART1); //复位串口1

	/* TX-PA9 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //TX PA9 推挽复用输出模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* RX-PA10  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //RX PA10浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound; //设置为19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式

	USART_Init(USART1, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //开启中断

	USART_Cmd(USART1, ENABLE); //使能串口
}

struct STM_UART_INFO : public UART_INFO {
	STM_UART_INFO(USART_TypeDef &puart) : huart(puart)
	{
	}
	USART_TypeDef &huart;

	unsigned char *txbuff = nullptr;
	int txpoint = 0;
	int txmax = 0;
	int txenable = 0;
	int rxenable = 0;

	osThreadId_t Thread_rcv = nullptr;
#define WAIT_RX (1 << 0)
#define WAIT_TX (1 << 1)
	virtual int send(unsigned char *buff, unsigned int num) final
	{
		if (buff && (num > 0)) {
			txenable = 1;
			txbuff = buff;
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

	virtual int rece(unsigned char *buff, unsigned int num) final
	{
		if (buff && (num > 0)) {
			rxenable = 1;
			rxbuff = buff;
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
		while (0 != checkTXCPL()) {
			osDelay(1);
		}
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

	virtual int open(int speed) final
	{
		return 0;
	}

	void usart_handle()
	{
		if (USART_GetITStatus(&huart, USART_IT_RXNE) != RESET) {
			int data = USART_ReceiveData(&huart); //(USART1->DR);	//读取接收到的数据

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

		if (USART_GetITStatus(&huart, USART_IT_TXE) == SET) {
			if ((txpoint < txmax) && (txbuff) && txenable) {
				USART_SendData(&huart, txbuff[txpoint++]);
				if (phandl)
					phandl->trig_tx();
			} else {
				txenable = 0;
				USART_ITConfig(&huart, USART_IT_TXE, DISABLE);
				if (phandl)
					phandl->trig_tx_cpl();
			}
		}

		if (USART_GetFlagStatus(&huart, USART_FLAG_ORE) == SET) //清除溢出错误
		{
			USART_ClearFlag(&huart, USART_FLAG_ORE);
			int Res = USART_ReceiveData(&huart);
		}
	}
};

/*
* link to hardware
*/

STM_UART_INFO stmuart5(*UART5);
STM_UART_INFO stmuart4(*UART4);
STM_UART_INFO stmuart3(*USART3);
STM_UART_INFO stmuart2(*USART2);
STM_UART_INFO stmuart1(*USART1);

extern "C" void UART5_IRQHandler(void)
{
	stmuart5.usart_handle();
}

extern "C" void UART4_IRQHandler(void)
{
	stmuart4.usart_handle();
}

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
	{ 4, &stmuart4 }, //绑定串口4
	{ 5, &stmuart5 }, //绑定串口5
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
