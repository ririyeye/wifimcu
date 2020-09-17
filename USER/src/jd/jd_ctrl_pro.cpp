
#include "jdframe.h"
#include <string.h>

#include "cc3200/cc3200.h"
#include "wifi/wifi_par.h"


typedef struct {
	int id;
	int (*pro)(FRAME_DATA *rec);
} PROFUN;
extern "C" void sendREC(FRAME_DATA *rec);

const uint16_t c0[][2] = {
	{ WIFI_SSID_0, WIFI_PWD_0 },
	{ WIFI_SSID_1, WIFI_PWD_1 },
	{ WIFI_SSID_2, WIFI_PWD_2 },
};
static unsigned char tmpbuff[128];

int set_id(FRAME_DATA *rec)
{
	unsigned char grpNum = rec->data[1];
	unsigned char *idname = &rec->data[2];
	int idlen = rec->len - MIN_PACK_SZ - 2;

	if (grpNum < 3) {
		if (idlen < 64) {
			WIFI_ParaTypeDef *pesp = getWIFI_PAR();
			int len = rec->len - MIN_PACK_SZ - 2;
			memcpy(pesp->WIFI_SSID[grpNum], idname, len);
			pesp->WIFI_SSID[grpNum][len] = 0;
			pesp->command_sta |= c0[grpNum][0];
		}
	}

	if (grpNum >= 3 && grpNum < 6) {
		if (idlen < 64) {
			WIFI_ParaTypeDef *pesp = getWIFI_PAR();
			int len = rec->len - MIN_PACK_SZ - 2;
			memcpy(pesp->WIFI_PWD[grpNum - 3], idname, len);
			pesp->WIFI_PWD[grpNum - 3][len] = 0;
			pesp->command_sta |= c0[grpNum - 3][1];
		}
	}
	return 1;
}

int get_id(FRAME_DATA *rec)
{
	unsigned char grpNum = rec->data[1];

	WIFI_ParaTypeDef *pesp = getWIFI_PAR();

	rec->data = tmpbuff;
	rec->data[1] = grpNum;
	if (grpNum < 3) {
		if (c0[grpNum][0] & pesp->command_sta) {
			rec->len = MIN_PACK_SZ + 2 + strlen((char *)pesp->WIFI_SSID[grpNum]);
			memcpy(&rec->data[2], pesp->WIFI_SSID[grpNum],
			       strlen((char *)pesp->WIFI_SSID[grpNum]));
		} else {
			rec->len = MIN_PACK_SZ + 2;
		}
	} else if (grpNum >= 3 && grpNum < 6) {
		if (c0[grpNum - 3][1] & pesp->command_sta) {
			rec->len = MIN_PACK_SZ + 2 + strlen((char *)pesp->WIFI_PWD[grpNum - 3]);
			memcpy(&rec->data[2], pesp->WIFI_PWD[grpNum - 3],
			       strlen((char *)pesp->WIFI_SSID[grpNum - 3]));

		} else {
			rec->len = MIN_PACK_SZ + 2;
		}
	}

	return 1;
}

int serverIP_PORT(FRAME_DATA *rec)
{
	int setlen = rec->len - MIN_PACK_SZ - 1;
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	if (setlen == 0) {
		rec->data = tmpbuff;
		unsigned char *datpos = &rec->data[1];
		memcpy(datpos, pesp->WIFI_remoteip, 4);
		datpos[4] = pesp->WIFI_tcpport;
		datpos[5] = pesp->WIFI_tcpport >> 8;
		rec->len = 6 + MIN_PACK_SZ + 1;
	} else if (setlen == 6) {
		unsigned char *datpos = &rec->data[1];
		memcpy(pesp->WIFI_remoteip, datpos, 4);
		pesp->WIFI_tcpport = datpos[4] | datpos[5] << 8;
		pesp->command_sta |= WIFI_REMOTE_IP_PORT;
	}
	return 1;
}

int LocalIP(FRAME_DATA *rec)
{
	int setlen = rec->len - MIN_PACK_SZ - 1;
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	if (setlen == 0) {
		rec->data = tmpbuff;
		unsigned char *datpos = &rec->data[1];
		memcpy(datpos + 0, pesp->WIFI_ip, 4);
		memcpy(datpos + 4, pesp->WIFI_gateway, 4);
		memcpy(datpos + 8, pesp->WIFI_netmask, 4);
		rec->len = 12 + MIN_PACK_SZ + 1;
	} else if (setlen == 12) {
		unsigned char *datpos = &rec->data[1];
		memcpy(pesp->WIFI_ip, datpos + 0, 4);
		memcpy(pesp->WIFI_gateway, datpos + 4, 4);
		memcpy(pesp->WIFI_netmask, datpos + 8, 4);
		pesp->command_sta |= WIFI_IP;
	}
	return 1;
}

int setconnect(FRAME_DATA *rec)
{
	int setlen = rec->len - MIN_PACK_SZ - 1;
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	if (setlen == 0) {
		pesp->command_sta |= WIFI_CONNECT;
	}
	return 1;
}

int setdisconnect(FRAME_DATA *rec)
{
	int setlen = rec->len - MIN_PACK_SZ - 1;
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	if (setlen == 0) {
		pesp->command_sta &= ~WIFI_CONNECT;
	}
	return 1;
}

int cacheStata(FRAME_DATA *rec)
{
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	rec->data = tmpbuff;
	unsigned char *datpos = &rec->data[1];
	char wup = 0, wdown = 0, wnull = 0;

	for (int i = 0; i < BUFFER_MAX_INDEX; i++) {
		if (pesp->buffer[i].flag == BUFFER_NULL) {
			wnull++;
		}
		if (pesp->buffer[i].flag == BUFFER_UPLOAD) {
			wup++;
		}
		if (pesp->buffer[i].flag == BUFFER_DOWNLOAD) {
			wdown++;
		}
	}
	datpos[0] = wup;
	datpos[1] = wdown;
	datpos[2] = wnull;
	rec->len = MIN_PACK_SZ + 1 + 3;
	return 1;
}
//重置通讯
static int cleanBuff(FRAME_DATA *rec)
{
	int setlen = rec->len - MIN_PACK_SZ - 1;
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	if (setlen == 0) {

		pesp->workStatus = STA_NULL;
	}
	return 1;
}
//设置地址
static int setmac(FRAME_DATA *rec)
{
	int setlen = rec->len - MIN_PACK_SZ - 1;
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	if (setlen == 4) {
		unsigned char *datpos = &rec->data[1];
		memcpy((void *)(pesp->thisdev), datpos, 4);
	}
	return 1;
}


const PROFUN ctrlgrp[] = {
	{ 0x40, set_id },
	{ 0x41, get_id },
	{ 0x42, serverIP_PORT },
	{ 0x43, LocalIP },
	{ 0x44, setconnect },
	{ 0x45, setdisconnect },

	{ 0x48, cacheStata },

	{ 0x4a, cleanBuff },

	{ 0x4b, setmac },

};

int ctrl_pro(FRAME_DATA *rec)
{
	for (auto &p : ctrlgrp) {
		if (p.id == rec->data[0]) {
			if (p.pro(rec)) {
				rec->data[0] = p.id | 0x80;
				sendREC(rec);
				return 1;
			}
			return 0;
		}
	}
	return 0;
}

int tim_pro(FRAME_DATA *rec)
{
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();

	switch (pesp->workStatus) {
	case STA_GET_TIM_OK:
		rec->data = tmpbuff;
		tmpbuff[0] = pesp->year;
		tmpbuff[1] = pesp->mon;
		tmpbuff[2] = pesp->day;
		tmpbuff[3] = pesp->hour;
		tmpbuff[4] = pesp->minute;
		tmpbuff[5] = pesp->sec;
		tmpbuff[6] = pesp->milsec;
		tmpbuff[7] = pesp->milsec >> 8;
		rec->len = 8 + MIN_PACK_SZ;
		pesp->workStatus = STA_NULL;
		return 1;
	default:
		pesp->workStatus = STA_GET_TIM;
		return 1;
	}
}

int num_pro(FRAME_DATA *rec)
{
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();

	switch (pesp->workStatus) {
	case STA_GET_NUM_OK:
		rec->data = tmpbuff;
		tmpbuff[0] = pesp->message_num[0];
		tmpbuff[1] = pesp->message_num[0] >> 8;
		tmpbuff[2] = pesp->message_num[1];
		tmpbuff[3] = pesp->message_num[1] >> 8;

		rec->len = 4 + MIN_PACK_SZ;
		pesp->workStatus = STA_NULL;
		return 1;
	default:
		pesp->workStatus = STA_GET_NUM;
		return 1;
	}
}
