#ifndef __wifi_par_H___
#define __wifi_par_H___

#include <stdint.h>



#ifdef __cplusplus
extern "C" {
#endif
#include "jd/jdframe.h"
	
typedef enum {
	BUFFER_NULL = 0,
	//code = 3
	BUFFER_ACQUIRE_DATA = 1,
	//code = 4
	BUFFER_UPLOAD = 2,
	//code = 5
	BUFFER_DOWNLOAD = 3,
} BUFF_STATUS;

typedef struct {
	unsigned char flag;
	FRAME_DATA fd;
	unsigned char dat[1024 - MIN_PACK_SZ];
} databuffer;

typedef enum {
	WIFI_SSID_0 = 1 << 0,
	WIFI_PWD_0 = 1 << 1,

	WIFI_SSID_1 = 1 << 2,
	WIFI_PWD_1 = 1 << 3,

	WIFI_SSID_2 = 1 << 4,
	WIFI_PWD_2 = 1 << 5,

	WIFI_IP = 1 << 6,

	WIFI_REMOTE_IP_PORT = 1 << 7,

	WIFI_CONNECT = 1 << 8,
} WIFI_FLG_MASK;

typedef enum {
	STA_NULL = 0,
	//获取时间
	STA_GET_TIM = 3,
	STA_GET_TIM_OK = 4,
	//获取编号
	STA_GET_NUM = 5,
	STA_GET_NUM_OK = 6,

	//
	STA_ACQUIRE_DATA = 7,
	STA_ACQUIRE_DATA_OK = 8,

	STA_SEND_DATA_SINGLE = 9,
	STA_SEND_DATA_SINGLE_OK = 10,

	STA_SEND_DATA_MULITY = 11,
	STA_SEND_DATA_MULITY_OK = 12,
	//
	STA_REC_DATA_SINGLE = 13,
	STA_REC_DATA_SINGLE_OK = 14,

	STA_REC_DATA_MULITY = 15,
	STA_REC_DATA_MULITY_OK = 16,
} CONNECT_STATUS;

typedef enum {
	AUTO_BUFFER_NULL = 0,
	AUTO_BUFFER_START_BUFFER = 1,
	AUTO_BUFFER_END_BUFFER = 2,
} AUTO_BUFFER_STATUS;

#define BUFFER_MAX_INDEX (1)
typedef struct {
	unsigned char WIFI_Error_retry_Cnt; //异常重试计数

	unsigned char workStatus;

	unsigned char WIFI_Mode;
	unsigned int WIFI_baud_rate;
	unsigned char WIFI_RFPOWER;
	unsigned char WIFI_SSID_Type; //WIFI连接SSID类型

	unsigned char WIFI_SSID[3][64]; //0:默认SSID  1:主SSID  2://备用SSID
	unsigned char WIFI_PWD[3][64];

	unsigned char WIFI_mac[6];
	unsigned char WIFI_ip[4];
	unsigned char WIFI_gateway[4];
	unsigned char WIFI_netmask[4];

	unsigned char WIFI_remoteip[4];
	unsigned int WIFI_tcpport;

	unsigned char WIFI_dhcp_param; // WIFI_DISABLE/WIFI_EABLE
	unsigned char WIFI_auto_connect; // WIFI_DISABLE/WIFI_EABLE

	uint16_t now_sta;
	uint16_t command_sta;

	uint16_t message_num[2];

	char year, mon, day, hour, minute, sec;
	uint16_t milsec;

	//本机ID
	char thisdev[4];

	databuffer buffer[BUFFER_MAX_INDEX];
	uint16_t auto_bufferring_index;
	char auto_buffer_status;
	//下载用 自动缓冲数据壳
	char dummyDownloadbuffer[16];
	char dummyDonwloadlen;
	FRAME_DATA dummyfd;
} WIFI_ParaTypeDef;


typedef int (search_meth)(databuffer *, void * pri);
WIFI_ParaTypeDef *getWIFI_PAR(void);
void wificleanBuffer(WIFI_ParaTypeDef *pesp);
databuffer *wifiSearchMethBuffer(WIFI_ParaTypeDef *pesp, search_meth method, void *pri);
databuffer *wifiSearchMinUploadIndexBuffer(WIFI_ParaTypeDef *pesp);
databuffer *wifiGetBuffer(WIFI_ParaTypeDef *pesp);
const char *getaimdev(void);

void timupdate_init(void);
#ifdef __cplusplus
}
#endif


#endif
