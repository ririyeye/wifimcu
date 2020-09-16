#ifndef __sbwt_h__
#define __sbwt_h__

#include "myuart.h"

struct SBWT_BUFF_STRUCT {
	unsigned char *txbuff;
	unsigned char *rxbuff;
	int rxdelay;
	int waitcpl;
};

struct SBWT_INFO {
	void init(UART_INFO *in_info, unsigned int newNum, SBWT_BUFF_STRUCT *inbuff)
	{
		if (in_info) {
			pinfo = in_info;
		}
		SetSerialNum(newNum);
		buff = inbuff;
	}

	enum sbwt_sta {
		sta_ok = 0,
		sta_error_pack = 1,

		sta_not_executed = 84,
		sta_unknown_com_data = 95,

		sta_not_init = 174,

		sta_unknuow = 10000,
	};

	int initTestSbwt();

	int scanfInit();

	int initSBWT();

	int GetDataSBWT();

	int SetData(int data1, int data2)
	{
		sbwt_data1 = data1;
		sbwt_data2 = data2;
		return 0;
	}

	int GetData(int index)
	{
		switch (index) {
		case 1:
			return sbwt_data1;
		case 2:
			return sbwt_data2;
		default:
			break;
		}
		return 0;
	}

	int ChangeNum(int newNum);
	int SetZeroCalibration();

	void SetSerialNum(unsigned int newNum)
	{
		SerialNum = newNum;
	}

	int getAlive()
	{
		return liveTime > 0;
	}
	void setAlive(int flag)
	{
		liveTime = flag;
	}

	unsigned char s_variant;
	unsigned char s_version[2];
	unsigned char h_version[2];
	unsigned char mlx_id[6];
	unsigned char mlx_soft_ver;
	unsigned char mlx_hard_ver;
	int sbwt_data1;
	int sbwt_data2;
	unsigned char SerialNum;

	int liveTime = 0;

    protected:
	UART_INFO *pinfo;
	SBWT_BUFF_STRUCT *buff;
};

struct TPack {
	unsigned char addr;
	unsigned char code;
	unsigned char *buff;
	unsigned int sz;
};

struct APPDECODE {
	unsigned int code;
	int (*funa)(TPack *tpack, SBWT_INFO &sbwt_session);
};

int checkAppData(TPack *pack, SBWT_INFO &session);

SBWT_INFO *GetSbwt_all(int index);
void dec_sbwt_group_liveTim(SBWT_INFO *grp, int maxindex, int decnum);
#endif
