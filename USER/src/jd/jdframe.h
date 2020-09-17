#ifndef __jdframe_H___
#define __jdframe_H___

#include <stdint.h>



#ifdef __cplusplus
extern "C" {
#endif

	
	typedef struct FRAME_DATA0{
		unsigned char code;
		unsigned char seq;
		unsigned char src_dev[4];
		unsigned char dst_dev[4];
		int16_t frame_index;
		uint16_t len;
		unsigned char * data;
		void * devInfo;
		uint16_t crc;
		void(*framedestroy)(struct FRAME_DATA0 * fd, void * dat);
		void * destorydat;
	}FRAME_DATA;

	enum
	{
		MIN_PACK_SZ = 18,//最小包长度
		MAX_PACK_SZ = 1036,//最大包长度
	};

	typedef enum {
		FRAME_SINGLE = -1,
		FRAME_NOT_READY = -2,
		FRAME_TRANSFER_COMPLETE = -3,
		FRAME_SAME = -4,
		FRAME_FULL = -5,
	}SP_FRAME;


	int pro_REC_DATA(FRAME_DATA * rec);
	int ctrl_pro(FRAME_DATA * rec);
	int tim_pro(FRAME_DATA * rec);
	int num_pro(FRAME_DATA * rec);
	int message_acquire_pro(FRAME_DATA * rec);
	int upload_pro(FRAME_DATA * rec);
	int download_pro(FRAME_DATA * rec);
	void sendREC(FRAME_DATA * rec);
	void mkSndBuf(unsigned char JDSndBuffer[], FRAME_DATA * rec , int downflg);
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus


#endif



#endif
