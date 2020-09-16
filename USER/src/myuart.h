#ifndef __myuart_h__
#define __myuart_h__
#ifdef __cplusplus
extern "C" {
#endif

struct UART_HANDLE {
	virtual void trig_rx()
	{
	}
	virtual void trig_rx_cpl()
	{
	}
	virtual void trig_tx()
	{
	}
	virtual void trig_tx_cpl()
	{
	}
};

struct UART_INFO {
	virtual int open(int speed) = 0;

	virtual int send(unsigned char *buff, unsigned int num) = 0;
	virtual int rece(unsigned char *buff, unsigned int num) = 0;
	virtual int wait_rece(unsigned int headtick, unsigned int tailtick) = 0;
	virtual int wait_send_end() = 0;

	virtual int checkTXCPL() = 0;
	virtual int checkRXCPL() = 0;

	virtual void stopTX() = 0;
	virtual void stopRX() = 0;

	virtual unsigned int GetTxNum() = 0;
	virtual unsigned int GetRxNum() = 0;

	void addHandle(UART_HANDLE *inhandle)
	{
		phandl = inhandle;
	}

    protected:
	UART_HANDLE *phandl;
};

UART_INFO *get_myuart(int index);

#ifdef __cplusplus
}
#endif

#endif
