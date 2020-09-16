
#ifndef __my_modbus__H__
#define __my_modbus__H__

#include "myuart.h"
#include <stdint.h>
//rtu

enum ModBus_proType {
	pro_stop = -5,
	pro_continue = -4,
	pro_send_continue = -3,

};

struct ModBus_rector {
	virtual int code() = 0;
	virtual int prodata(unsigned char *indata, int len, unsigned char *outbuff,
			    unsigned int outbufflen) = 0;
};

struct myModBus {
	void init(unsigned char inaddr,UART_INFO * in_info)
	{
		addr = inaddr;
		pinfo = in_info;
	}
	void poll_main();
	virtual int proRec(unsigned char *buff, unsigned int len, unsigned char *outbuff,
			   unsigned int outbufflen) = 0;

    protected:
	unsigned char addr;

    private:
	unsigned char txbuff[256];
	unsigned char rxbuff[256];
	UART_INFO *pinfo;
};

struct ModBus_RTU : public myModBus {
	void setRectorGroup(ModBus_rector *, int sz);
	virtual int proRec(unsigned char *buff, unsigned int len, unsigned char *outbuff,
			   unsigned int outbufflen) final;

    protected:
	ModBus_rector *grp = 0;
	int grpsz = 0;
};

#endif
