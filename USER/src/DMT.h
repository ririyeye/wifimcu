#include "myuart.h"
#include "cmsis_os2.h"
#include "sbwt.h"

struct DMT_UART_DATA_STRUCT {
	enum DMT_UART_STA {
		dmt_write = 0x82,
		dmt_read = 0x83,
	} dmt_sta;
	uint16_t start_addr;
	void *dat;
	unsigned char datlen;
};

#define USING_DMT_CRC 1
#define PER_PAGE_NUM 12
#define MAX_PAGE_NUM 4

int dmt_unpack_buff(DMT_UART_DATA_STRUCT *p, unsigned char *inbuff, int len, int using_crc);

int dmt_mk_buff(unsigned char *uartbufftx, DMT_UART_DATA_STRUCT *dat);

struct DMT {
	enum DMT_status {
		DMT_OK = 0,
		DMT_write_error = 1,
		DMT_read_error = 2,
		DMT_uart_not_init = 3,
		DMT_len_error = 4,
	};

	DMT_status write_data(int addr, void *buff, int &len);
	DMT_status read_data(int addr, void *buff, int &len);

	DMT_status write_int(int addr, int data);
	DMT_status write_float(int addr, float data);

	void init(UART_INFO *inuart);
	void rec_thread();
	void setuart(UART_INFO *inuart)
	{
		uartinfo = inuart;
	}
	int getshowIndex()
	{
		return showindex;
	}
	void SP_Work(UART_INFO *spuart, SBWT_BUFF_STRUCT *pbuf, unsigned int index);
	unsigned int get_page_num()
	{
		return pageindex;
	}

    private:
	UART_INFO *uartinfo;
	unsigned char uartbufftx[256];
	unsigned char uartbuffrx[256];

	int default_rec(DMT_UART_DATA_STRUCT *pdat);
	int write_rec(DMT_UART_DATA_STRUCT *pdat);

	typedef int (*protype)(void *p, DMT_UART_DATA_STRUCT *pdat);

	int showindex = -1;
	unsigned int pageindex = 0;

	protype *profun;
	void *proData;
	osThreadId_t write_id;

	int set_pro(protype fun, void *fundata);
	void read_addr_num(int addr, int value);

	int changeFlag[2] = { 0 };
	int correctFlag[2] = { 0 };

	int using_crc = USING_DMT_CRC;
};

enum SPWORK_UART_TYPE {
	spwork_uart_485 = 0,
	spwork_uart_lora = 1,
};

#define SPWORK_UART_MAX_TYPE_ID spwork_uart_lora

void dmt_ctl(UART_INFO *spuart, SBWT_BUFF_STRUCT *pbuf, unsigned int index);
