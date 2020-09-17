#include "jd/jdframe.h"
#include <string.h>
#include "cc3200/cc3200.h"
#include "wifi/wifi_par.h"

static unsigned char tmpbuffer[64];
void unflgBuffer(FRAME_DATA * pfd, void *p)
{
	auto * pesp = getWIFI_PAR();
	uint32_t index = (int)p;
	if (index < BUFFER_MAX_INDEX) {
		pesp->buffer[index].flag = 0;
	}
}

static int GetAimDataBuffer(databuffer * pbuff, void * p)
{
	int num = (int)p;
	if (pbuff->flag == BUFFER_DOWNLOAD) {
		if (pbuff->fd.frame_index == num) {
			return 1;
		}
	}
	return 0;
}

static int GetAimUploadDataBuffer(databuffer * pbuff, void * p)
{
	int num = (int)p;
	if (pbuff->flag == BUFFER_UPLOAD) {
		if (pbuff->fd.frame_index == num) {
			return 1;
		}
	}
	return 0;
}



int CleanLowerDataBuffer(databuffer * pbuff, void * p)
{
	//清除所有编号低于num的buffer
	int num = (int)p;
	if (pbuff->flag == BUFFER_DOWNLOAD) {
		if (pbuff->fd.frame_index < num) {
			pbuff->flag = 0;
		}
	}
	return 0;
}


int message_acquire_pro(FRAME_DATA * rec)
{
	auto * pesp = getWIFI_PAR();

	switch (pesp->workStatus) {

	case STA_ACQUIRE_DATA_OK:
		if (pesp->buffer[0].flag) {
			memcpy(rec, &pesp->buffer[0].fd, sizeof(FRAME_DATA));
			rec->data = pesp->buffer[0].dat;
			//完成标志
			rec->frame_index = FRAME_SINGLE;
		}
		pesp->workStatus = STA_NULL;
		rec->framedestroy = unflgBuffer;
		rec->destorydat = 0;
		return 1;
	default:
		pesp->workStatus = STA_ACQUIRE_DATA;
		pesp->message_num[0] = rec->data[0] | rec->data[1] << 8;
		wificleanBuffer(pesp);
		rec->frame_index = FRAME_NOT_READY;
		return 1;
	}
}

int upload_pro(FRAME_DATA * rec)
{
	auto * pesp = getWIFI_PAR();

	switch (pesp->workStatus) {

	case STA_SEND_DATA_SINGLE_OK:
		wificleanBuffer(pesp);
		pesp->workStatus = STA_NULL;
		rec->framedestroy = 0;
		rec->destorydat = 0;
		//发送完成
		rec->frame_index = FRAME_SINGLE;
		rec->data = 0;
		rec->len = MIN_PACK_SZ;
		return 1;
	case STA_SEND_DATA_MULITY:
		rec->framedestroy = 0;
		rec->destorydat = 0;
		//结束命令
		if (rec->frame_index == FRAME_TRANSFER_COMPLETE) {
			wificleanBuffer(pesp);
			pesp->workStatus = STA_NULL;
			//发送完成
			rec->frame_index = FRAME_TRANSFER_COMPLETE;
			rec->data = 0;
			rec->len = MIN_PACK_SZ;
			return 1;
		} else if (rec->frame_index >= 0) {
			if (rec->len > MIN_PACK_SZ) {
				//获取同index buffer
				databuffer * pbuffer = wifiSearchMethBuffer(pesp, GetAimUploadDataBuffer, (void *)rec->frame_index);
				if (!pbuffer) {
					//获取空buffer
					pbuffer = wifiGetBuffer(pesp);
					if (pbuffer) {
						memcpy(&pbuffer->fd, rec, sizeof(FRAME_DATA));
						memcpy(pbuffer->dat, rec->data, rec->len - MIN_PACK_SZ);
						pbuffer->flag = BUFFER_UPLOAD;
					} else {
						rec->frame_index = FRAME_FULL;
					}
				} else {
					rec->frame_index = FRAME_SAME;
				}
			}
			tmpbuffer[0] = 0;
			rec->data = tmpbuffer;
			rec->len = MIN_PACK_SZ + 1;
			return 1;
		}
	default:
		//单帧指令
		if (rec->frame_index == FRAME_SINGLE) {
			wificleanBuffer(pesp);
			memcpy(&pesp->buffer[0].fd, rec, sizeof(FRAME_DATA));
			memcpy(&pesp->buffer[0].dat, rec->data, rec->len - MIN_PACK_SZ);
			pesp->buffer[0].flag = BUFFER_UPLOAD;
			//尚未发送完成
			rec->frame_index = FRAME_NOT_READY;
			rec->data = 0;
			rec->len = MIN_PACK_SZ;
			pesp->workStatus = STA_SEND_DATA_SINGLE;
		} else {
			databuffer * pbuffer = &pesp->buffer[0];
			wificleanBuffer(pesp);
			memcpy(&pbuffer->fd, rec, sizeof(FRAME_DATA));
			memcpy(pbuffer->dat, rec->data, rec->len - MIN_PACK_SZ);
			pbuffer->flag = BUFFER_UPLOAD;
			pesp->workStatus = STA_SEND_DATA_MULITY;
		}
		return 1;
	}
}




static void start_auto_buffer(WIFI_ParaTypeDef * pesp , FRAME_DATA * rec,int index)
{
	wificleanBuffer(pesp);
	pesp->auto_buffer_status = AUTO_BUFFER_START_BUFFER;
	pesp->auto_bufferring_index = index;
	//记录自动缓冲壳信息
	memcpy(pesp->dummyDownloadbuffer, rec->data, rec->len - MIN_PACK_SZ);
	pesp->dummyDonwloadlen = rec->len - MIN_PACK_SZ;
	memcpy(&pesp->dummyfd, rec, sizeof(FRAME_DATA));
	//多帧自动模式
	pesp->workStatus = STA_REC_DATA_MULITY;
}


int download_pro(FRAME_DATA * rec)
{
    auto * pesp = getWIFI_PAR();

	switch (pesp->workStatus) {

	case STA_REC_DATA_SINGLE:
		if (pesp->buffer[0].flag) {
			memcpy(rec, &pesp->buffer[0].fd, sizeof(FRAME_DATA));
			rec->data = pesp->buffer[0].dat;
			//完成标志
			rec->frame_index = FRAME_SINGLE;
		}
		pesp->workStatus = STA_NULL;
		rec->framedestroy = unflgBuffer;
		rec->destorydat = 0;
		return 1;
	case STA_REC_DATA_SINGLE_OK:
		wificleanBuffer(pesp);
		pesp->workStatus = STA_NULL;
		memcpy(rec, &pesp->buffer[0].fd, sizeof(FRAME_DATA));
		rec->data = pesp->buffer[0].dat;
		rec->framedestroy = 0;
		rec->destorydat = 0;
		return 1;

	case STA_REC_DATA_MULITY:
		if (rec->frame_index > pesp->auto_bufferring_index) {
			//启动auto buffer
			start_auto_buffer(pesp, rec, rec->frame_index);
			//返回等待
			rec->frame_index = FRAME_NOT_READY;
			rec->data = 0;
			rec->len = MIN_PACK_SZ;
		} else {
			databuffer * pbuffer = wifiSearchMethBuffer(pesp, GetAimDataBuffer, (void *)rec->frame_index);
			if (pbuffer) {
				wifiSearchMethBuffer(pesp, CleanLowerDataBuffer, (void *)rec->frame_index);
				memcpy(rec, &pbuffer->fd, sizeof(FRAME_DATA));
				rec->data = pbuffer->dat;
				rec->framedestroy = 0;
				rec->destorydat = 0;
			} else {
				//返回等待
				rec->frame_index = FRAME_NOT_READY;
				rec->data = 0;
				rec->len = MIN_PACK_SZ;
				rec->framedestroy = 0;
				rec->destorydat = 0;
			}
		}
		break;
	case STA_REC_DATA_MULITY_OK:
		if (rec->frame_index == pesp->auto_bufferring_index) {
			//终止帧
			wificleanBuffer(pesp);
			pesp->workStatus = STA_NULL;
			memcpy(rec, &pesp->buffer[0].fd, sizeof(FRAME_DATA));
			rec->data = 0;
			rec->framedestroy = 0;
			rec->destorydat = 0;
			rec->len = MIN_PACK_SZ;
			//发送完成
			rec->frame_index = FRAME_TRANSFER_COMPLETE;
		} else {
			databuffer * pbuffer = wifiSearchMethBuffer(pesp, GetAimDataBuffer, (void *)rec->frame_index);
			if (pbuffer) {
				//返回缓冲
				memcpy(rec, &pbuffer->fd, sizeof(FRAME_DATA));
				rec->data = pbuffer->dat;
				rec->framedestroy = 0;
				rec->destorydat = 0;
			} else {
				//返回等待
				rec->frame_index = FRAME_NOT_READY;
				rec->data = 0;
				rec->len = MIN_PACK_SZ;
				rec->framedestroy = 0;
				rec->destorydat = 0;
			}
		}
		return 1;
	default:
		//单帧指令
		if (rec->frame_index == 0) {
			//停止auto buffer
			wificleanBuffer(pesp);
			pesp->auto_buffer_status = AUTO_BUFFER_NULL;
			//添加发送数据
			memcpy(&pesp->buffer[0].fd, rec, sizeof(FRAME_DATA));
			memcpy(&pesp->buffer[0].dat, rec->data, rec->len - MIN_PACK_SZ);
			//单帧模式
			pesp->workStatus = STA_REC_DATA_SINGLE;
		} else if (rec->frame_index > 0) {
			//启动auto buffer
			start_auto_buffer(pesp, rec, rec->frame_index);
		}
		//返回等待
		rec->frame_index = -2;
		rec->data = 0;
		rec->len = MIN_PACK_SZ;
		return 1;
	}
	return 1;
}

