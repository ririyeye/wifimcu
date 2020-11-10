#include "cmsis_os2.h"
#include "myuart.h"
#include "stdio.h"
#include "stm32f10x.h"
#include <string.h>
#include "cc3200/cc3200.h"
#include "wifi/wifi_par.h"
template <typename T, size_t N> inline size_t CountOf(T (&arr)[N])
{
	return N;
}

static uint64_t thread_cc3200_stk[64];

void cc3200_main(void *argument);

void init_cc3200(void)
{
	osThreadAttr_t thread1_attr;
	memset(&thread1_attr, 0, sizeof(osThreadAttr_t));
	thread1_attr.stack_mem = &thread_cc3200_stk[0];
	thread1_attr.stack_size = sizeof(thread_cc3200_stk);
	thread1_attr.priority = osPriorityHigh1;
	osThreadId_t ost = osThreadNew(&cc3200_main, NULL, &thread1_attr);
}

static unsigned char txbuff[1100];
static unsigned char rxbuff[1100];

void cc3200_gpio_ctrl_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //开启U2串口时钟
	//reset io
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//pwr io
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void cc3200_pwr(bool sta)
{
	if (sta) {
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
		GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_SET);
	} else {
		GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_RESET);
	}
}

void cc3200_hw_force_reset_cfg(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_RESET);
	osDelay(2000);
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, Bit_SET);
}

int checkATmod(char *rxbuff, int size)
{
	if (strstr(rxbuff, "Entered AT")) {
		return 0;
	}

	if (strstr(rxbuff, "Unable to")) {
		return 0;
	}
	return -1;
}

int enter_at_mode(UART_INFO *info, int *speedgrp, int speedcnt)
{
	int txnum = sprintf((char *)txbuff, "+++");

	for (int retrycnt = 0; retrycnt < 100; retrycnt++) {
		info->opts->setspeed(speedgrp[retrycnt % speedcnt]);

		osDelay(100);

		info->send(txbuff, txnum);
		info->rece(rxbuff, sizeof(rxbuff));
		info->wait_rece(200, 200);

		if (info->GetRxNum() > 0 && !checkATmod((char *)rxbuff, info->GetRxNum())) {
			return 0;
		}
	}
	return -1;
}

int get_ssid(UART_INFO *info, char *ssid, char *passwd)
{
	for (size_t i = 0; i < 10; i++) {
		char *cmd = "AT+STA=?\r\n";
		info->send(cmd, strlen(cmd));
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, 200);

		int cnt;
		int scnum = sscanf((char *)rxbuff, "SSID:%s TYPE:%d Password:%s\r\n", ssid, &cnt,
				   passwd);

		if (scnum > 0) {
			return 0;
		}
	}
	return -1;
}

int set_ssid(UART_INFO *info, char *ssid, char *passwd)
{
	int snlen = 0;
	if (passwd) {
		snlen = sprintf((char *)txbuff, "AT+STA=%s,2,%s\r\n", ssid, passwd);
	} else {
		snlen = sprintf((char *)txbuff, "AT+STA=%s,0\r\n", ssid);
	}

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "STA update OK")) {
			return 0;
		}
	}
	return -1;
}

int set_mod_sta(UART_INFO *info)
{
	for (size_t i = 0; i < 10; i++) {
		char *cmd = "AT+ROLE=STA\r\n";
		info->send(cmd, strlen(cmd));
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(200, 200);

		if (info->GetRxNum() && strstr((char *)rxbuff, "Set STA mode")) {
			return 0;
		}
	}
	return -1;
}

int set_fix_ip_port(UART_INFO *info, const char *staip, const char *mask, const char *gatewayip)
{
	int snlen = 0;
#if 1
	snlen = sprintf((char *)txbuff, "AT+STAIP=STATIC,%s,%s,%s,%s\r\n", staip, mask, gatewayip,
			"223.5.5.5");
#else
	snlen = sprintf((char *)txbuff, "AT+STAIP=STATIC,%s,%s,%s,%s\r\n", staip, mask, "192.168.98.1",
			"223.5.5.5");
#endif
	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "STAIP update OK")) {
			return 0;
		}
	}
	return -1;
}

int set_server_ip_port(UART_INFO *info, const char *serverip, int port)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+SOCK=TCP,CLIENT,%s,%d,1234\r\n", serverip, port);

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Socket update OK")) {
			return 0;
		}
	}
	return -1;
}

int set_exit(UART_INFO *info)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+EXIT\r\n");

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Exited AT mode")) {
			return 0;
		}
	}
	return -1;
}

int set_PM(UART_INFO *info, int mod)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+PM=%d,240\r\n", mod);

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Power mode set OK")) {
			return 0;
		}
	}
	return -1;
}

int set_uart_speed(UART_INFO *info, int speed)
{
	int snlen = 0;
	snlen = sprintf((char *)txbuff, "AT+UART=%d,8,0,1\r\n", speed);

	for (size_t i = 0; i < 10; i++) {
		info->send((char *)txbuff, snlen);
		info->rece(rxbuff, CountOf(rxbuff));
		info->wait_rece(1000, 200);

		if (strstr((char *)rxbuff, "Uart update OK")) {
			return 0;
		}
	}
	return -1;
}

int chkNeedConnect(WIFI_ParaTypeDef *pesp)
{
	const uint16_t connectgrp[] = {
		WIFI_IP,
		WIFI_REMOTE_IP_PORT,
		WIFI_CONNECT,
	};

	for (int i = 0; i < sizeof(connectgrp) / sizeof(uint16_t); i++) {
		if (connectgrp[i] & pesp->command_sta) {
		} else {
			return -1;
		}
	}

	const uint16_t c0[][2] = {
		{ WIFI_SSID_0, WIFI_PWD_0 },
		{ WIFI_SSID_1, WIFI_PWD_1 },
		{ WIFI_SSID_2, WIFI_PWD_2 },
	};

	for (int i = 0; i < 3; i++) {
		if (pesp->command_sta & (c0[i][0] | c0[i][1])) {
			return i;
		}
	}
	return -1;
}
static unsigned char tmpbuff[128];
static unsigned char seq = 0;

void mkProtocolTim(FRAME_DATA *pfd)
{
	pfd->code = 0x01;
	pfd->seq = seq++;
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	memcpy(pfd->src_dev, pesp->thisdev, 4);
	memcpy(pfd->dst_dev, getaimdev(), 4);
	pfd->frame_index = 0;
	pfd->data = tmpbuff;
	pfd->len = MIN_PACK_SZ;
	pfd->framedestroy = 0;
}

void mkProtocolGetNum(FRAME_DATA *pfd)
{
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	pfd->code = 0x02;
	pfd->seq = seq++;
	memcpy(pfd->src_dev, pesp->thisdev, 4);
	memcpy(pfd->dst_dev, getaimdev(), 4);
	pfd->frame_index = 0;
	pfd->data = tmpbuff;
	pfd->len = MIN_PACK_SZ;
	pfd->framedestroy = 0;
}

void mkAcquireIndex(FRAME_DATA *pfd, uint16_t indexID)
{
	WIFI_ParaTypeDef *pesp = getWIFI_PAR();
	pfd->code = 0x03;
	pfd->seq = seq++;
	memcpy(pfd->src_dev, pesp->thisdev, 4);
	memcpy(pfd->dst_dev, getaimdev(), 4);
	pfd->frame_index = 0;
	pfd->data = tmpbuff;
	tmpbuff[0] = indexID;
	tmpbuff[1] = indexID >> 8;
	pfd->len = MIN_PACK_SZ + 2;
	pfd->framedestroy = 0;
}

void mkSendData(FRAME_DATA *pfd, databuffer *pbuffer)
{
	pfd->code = 0x04;
	memcpy(pfd, &pbuffer->fd, sizeof(FRAME_DATA));
	pfd->seq = seq++;
	pfd->data = pbuffer->dat;
	pfd->framedestroy = 0;
}

void mkRecvDataSingle(FRAME_DATA *pfd, databuffer *pbuffer)
{
	pfd->code = 0x05;
	memcpy(pfd, &pbuffer->fd, sizeof(FRAME_DATA));
	pfd->seq = seq++;
	pfd->data = pbuffer->dat;
	pfd->frame_index = 0;
	pfd->framedestroy = 0;
}

void mkRecvDataMulity(FRAME_DATA *pfd, FRAME_DATA *pdummyfd, int index, unsigned char *pdummybuffer,
		      int len)
{
	memcpy(pfd, pdummyfd, sizeof(FRAME_DATA));
	pfd->seq = seq++;
	pfd->code = 0x05;
	pfd->data = pdummybuffer;
	pfd->len = MIN_PACK_SZ + len;
	pfd->frame_index = index;
	pfd->framedestroy = 0;
}

int exchangeTCPdatOnce(UART_INFO *pwifi, FRAME_DATA *pfd)
{
	mkSndBuf(txbuff, pfd, 0);
	pwifi->send(txbuff, pfd->len);
	pwifi->wait_send_end();
	pwifi->rece(rxbuff, sizeof(rxbuff));
	pwifi->wait_rece(1000, 2);

	return pwifi->GetRxNum() > 0 ? 0 : -1;
}

int downloadlargepack(UART_INFO *pwifi, FRAME_DATA *pfd)
{
	mkSndBuf(txbuff, pfd, 0);
	pwifi->send(txbuff, pfd->len);
	pwifi->wait_send_end();
	pwifi->rece(rxbuff, sizeof(rxbuff));
	pwifi->wait_rece(1000, 30);

	return pwifi->GetRxNum() > 0 ? pwifi->GetRxNum() : -1;
}

int SendTCPdatOnce(UART_INFO *pwifi, FRAME_DATA *pfd)
{
	pwifi->wait_send_end();
	mkSndBuf(txbuff, pfd, 0);
	pwifi->send(txbuff, pfd->len);
	pwifi->wait_send_end();
	osDelay(20);
	return 0;
}

void set_live_led(int sta);
void set_com_led(int sta);

void mk_REC_DATA(unsigned char *data, int len, FRAME_DATA *rec)
{
	if (!rec) {
		return;
	}

	rec->code = data[2];
	rec->seq = data[3];
	memcpy(rec->dst_dev, data + 4, 4);
	memcpy(rec->src_dev, data + 8, 4);

	rec->frame_index = data[12] | data[13] << 8;
	rec->len = data[14] | data[15] << 8;
	rec->data = data + 16;

	rec->crc = data[len - 2] | data[len - 1] << 8;

	rec->framedestroy = 0;
}

int speeds[] = { 115200 * 1, 115200 * 10 };
int JD_Communication_data(unsigned char *rxbuf, int num, PROTOCOL_DAT *ret);
void cc3200_main(void *argument)
{
	//power off
	cc3200_gpio_ctrl_init();

	while (true) {
		cc3200_pwr(0);

		WIFI_ParaTypeDef *pesp = getWIFI_PAR();

		UART_INFO *pwifi = get_myuart(1);
		int connectindex;
		do {
			connectindex = chkNeedConnect(pesp);
			osDelay(100);
			set_live_led(0);
		} while (0 > connectindex);
		cc3200_pwr(1);

		enter_at_mode(pwifi, speeds, CountOf(speeds));

		//skip set mac

		set_mod_sta(pwifi);

		{
			char buff[64];
			sprintf(buff, "try connect %s\n", (char *)pesp->WIFI_SSID[connectindex]);
			//debug_sndstring(buff);
			set_ssid(pwifi, (char *)pesp->WIFI_SSID[connectindex],
				 (char *)pesp->WIFI_PWD[connectindex]);
		}

		{
			char buff[3][20];
			sprintf(buff[0], "%d.%d.%d.%d", pesp->WIFI_ip[0], pesp->WIFI_ip[1],
				pesp->WIFI_ip[2], pesp->WIFI_ip[3]);
			sprintf(buff[1], "%d.%d.%d.%d", pesp->WIFI_gateway[0],
				pesp->WIFI_gateway[1], pesp->WIFI_gateway[2],
				pesp->WIFI_gateway[3]);
			sprintf(buff[2], "%d.%d.%d.%d", pesp->WIFI_netmask[0],
				pesp->WIFI_netmask[1], pesp->WIFI_netmask[2],
				pesp->WIFI_netmask[3]);
			//debug_sndstring(buff);
			set_fix_ip_port(pwifi, buff[0], buff[2], buff[1]);
		}

		{
			char buff[32];
			sprintf(buff, "%d.%d.%d.%d", pesp->WIFI_remoteip[0], pesp->WIFI_remoteip[1],
				pesp->WIFI_remoteip[2], pesp->WIFI_remoteip[3]);
			set_server_ip_port(pwifi, buff, pesp->WIFI_tcpport);
			sprintf(buff, "server %d.%d.%d.%d:%d", pesp->WIFI_remoteip[0],
				pesp->WIFI_remoteip[1], pesp->WIFI_remoteip[2],
				pesp->WIFI_remoteip[3], pesp->WIFI_tcpport);
			//debug_sndstring(buff[0]);
		}

		set_PM(pwifi, 0);

		set_uart_speed(pwifi, speeds[CountOf(speeds) - 1]);

		set_exit(pwifi);

		pwifi->opts->setspeed(speeds[CountOf(speeds) - 1]);
		//reset cc3200
		cc3200_pwr(0);
		osDelay(100);
		cc3200_pwr(1);
		//wait power on
		osDelay(2000);
		char failtim = 0;
		while (true) {
			FRAME_DATA fdata;
			PROTOCOL_DAT dat;
			char buff[64];
			//对时阶段
			if (pesp->workStatus == STA_GET_TIM) {
				mkProtocolTim(&fdata);
				if (0 == exchangeTCPdatOnce(pwifi, &fdata) &&
				    0 == JD_Communication_data(rxbuff, pwifi->GetRxNum(), &dat)) {
					//检测到JD返回帧
					failtim = 0;
					mk_REC_DATA(dat.datestart, dat.datelen, &fdata);
					if (fdata.len == (MIN_PACK_SZ + 8)) {
						//检查时间
						pesp->year = fdata.data[0];
						pesp->mon = fdata.data[1];
						pesp->day = fdata.data[2];
						pesp->hour = fdata.data[3];
						pesp->minute = fdata.data[4];
						pesp->sec = fdata.data[5];
						pesp->milsec = fdata.data[6] | (fdata.data[7] << 8);
						pesp->workStatus = STA_GET_TIM_OK;
						//debug_sndstring("code = 1,get time\n");
					}
				}
			}
			//编号获取
			if (pesp->workStatus == STA_GET_NUM) {
				mkProtocolGetNum(&fdata);
				if (0 == exchangeTCPdatOnce(pwifi, &fdata) &&
				    0 == JD_Communication_data(rxbuff, pwifi->GetRxNum(), &dat)) {
					//检测到JD返回帧
					failtim = 0;
					mk_REC_DATA(dat.datestart, dat.datelen, &fdata);
					if (fdata.len == (MIN_PACK_SZ + 4)) {
						//检查编号
						pesp->message_num[0] =
							fdata.data[0] | (fdata.data[1] << 8);
						pesp->message_num[1] =
							fdata.data[2] | (fdata.data[3] << 8);
						pesp->workStatus = STA_GET_NUM_OK;
						sprintf(buff, "code = 2,index from %d to %d\n",
							pesp->message_num[0], pesp->message_num[1]);
						//debug_sndstring(buff);
					}
				}
			}
			//信件获取
			if (pesp->workStatus == STA_ACQUIRE_DATA) {
				mkAcquireIndex(&fdata, pesp->message_num[0]);
				if (0 == exchangeTCPdatOnce(pwifi, &fdata) &&
				    0 == JD_Communication_data(rxbuff, pwifi->GetRxNum(), &dat)) {
					//检测到JD返回帧
					failtim = 0;
					mk_REC_DATA(dat.datestart, dat.datelen, &fdata);
					if (fdata.len > (MIN_PACK_SZ)) {
						//复制数据
						memcpy(&pesp->buffer[0].fd, &fdata,
						       sizeof(FRAME_DATA));
						memcpy(pesp->buffer[0].dat, fdata.data,
						       fdata.len - MIN_PACK_SZ);
						pesp->buffer[0].flag = BUFFER_ACQUIRE_DATA;
						pesp->workStatus = STA_ACQUIRE_DATA_OK;
						if (fdata.len > MIN_PACK_SZ + 2) {
							sprintf(buff,
								"code = 3,index = %d,msgid = %d,subid=%d\n",
								fdata.frame_index,
								fdata.data[0] |
									(fdata.data[1] << 8),
								fdata.data[2]);
							//debug_sndstring(buff);
						}
					}
				}
			}

			//数据发送
			//单帧 不需要返回
			if (pesp->workStatus == STA_SEND_DATA_SINGLE) {
				mkSendData(&fdata, &pesp->buffer[0]);
				SendTCPdatOnce(pwifi, &fdata);
				pesp->workStatus = STA_SEND_DATA_SINGLE_OK;
				sprintf(buff, "code = 4,index = %d,msgid = %d,subid=%d\n",
					fdata.frame_index, fdata.data[0] | (fdata.data[1] << 8),
					fdata.data[2]);
				//debug_sndstring(buff);
			}
			//多帧发送
			if (pesp->workStatus == STA_SEND_DATA_MULITY) {
				databuffer *pbuffer = wifiSearchMinUploadIndexBuffer(pesp);
				if (pbuffer) {
					mkSendData(&fdata, pbuffer);
					sprintf(buff, "code = 4,index = %d,msgid = %d,subid=%d\n",
						fdata.frame_index,
						fdata.data[0] | (fdata.data[1] << 8),
						fdata.data[2]);
					//debug_sndstring(buff);
					if (0 == exchangeTCPdatOnce(pwifi, &fdata) &&
					    0 == JD_Communication_data(rxbuff, pwifi->GetRxNum(),
								       &dat)) {
						mk_REC_DATA(dat.datestart, dat.datelen, &fdata);

						if (fdata.frame_index == pbuffer->fd.frame_index) {
							pbuffer->flag = 0;
						}
					}
				}
			}

			//数据接收
			//单帧
			if (pesp->workStatus == STA_REC_DATA_SINGLE) {
				mkRecvDataSingle(&fdata, &pesp->buffer[0]);
				if (0 == exchangeTCPdatOnce(pwifi, &fdata) &&
				    0 == JD_Communication_data(rxbuff, pwifi->GetRxNum(), &dat)) {
					//检测到JD返回帧
					failtim = 0;
					mk_REC_DATA(dat.datestart, dat.datelen, &fdata);
					if (fdata.len > (MIN_PACK_SZ)) {
						//复制数据
						memcpy(&pesp->buffer[0].fd, &fdata,
						       sizeof(FRAME_DATA));
						memcpy(pesp->buffer[0].dat, fdata.data,
						       fdata.len - MIN_PACK_SZ);
						pesp->buffer[0].flag = BUFFER_DOWNLOAD;
						pesp->workStatus = STA_REC_DATA_SINGLE_OK;
						sprintf(buff,
							"code = 5,index = %d,msgid = %d,subid=%d\n",
							fdata.frame_index,
							fdata.data[0] | (fdata.data[1] << 8),
							fdata.data[2]);
						//debug_sndstring(buff);
					}
				}
			}
			//多帧
			if (pesp->workStatus == STA_REC_DATA_MULITY) {
				databuffer *pbuff = wifiGetBuffer(pesp);
				if (pbuff &&
				    (pesp->auto_buffer_status == AUTO_BUFFER_START_BUFFER)) {
					mkRecvDataMulity(&fdata, &pesp->dummyfd,
							 pesp->auto_bufferring_index,
							 (unsigned char *)pesp->dummyDownloadbuffer,
							 pesp->dummyDonwloadlen);

					int reclen = downloadlargepack(pwifi, &fdata);
					if (reclen > 0 &&
					    0 == JD_Communication_data(rxbuff, reclen, &dat)) {
						//检测到JD返回帧
						failtim = 0;
						mk_REC_DATA(dat.datestart, dat.datelen, &fdata);
						sprintf(buff,
							"code = 5,index = %d,msgid = %d,subid=%d\n",
							fdata.frame_index,
							fdata.data[0] | (fdata.data[1] << 8),
							fdata.data[2]);
						//debug_sndstring(buff);
						if ((fdata.len > MIN_PACK_SZ) &&
						    (fdata.frame_index ==
						     pesp->auto_bufferring_index)) {
							//复制数据
							memcpy(&pbuff->fd, &fdata,
							       sizeof(FRAME_DATA));
							memcpy(pbuff->dat, fdata.data,
							       fdata.len - MIN_PACK_SZ);
							pbuff->flag = BUFFER_DOWNLOAD;
							//开始下一帧缓冲
							pesp->auto_bufferring_index++;
						} else if (fdata.len == MIN_PACK_SZ) {
							//缓冲完成
							pesp->auto_buffer_status =
								AUTO_BUFFER_END_BUFFER;
							pesp->workStatus = STA_REC_DATA_MULITY_OK;
						}
					}
				}
			}
			osDelay(1);
			if (chkNeedConnect(pesp) < 0) {
				break;
			}
			if (osKernelGetTickCount() / 1000 % 2) {
				set_com_led(1);
			} else {
				set_com_led(0);
			}
		}
	}
}
