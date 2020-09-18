
#include "jd/jdframe.h"
#include "myuart.h"
#include <string.h>

//#include "ESP8266WIFI.h"

int pro_REC_DATA(UART_INFO *pinfo, FRAME_DATA *rec)
{
	char ret = 0;

	switch (rec->code) {
	case 0x00:
		ret = ctrl_pro(rec);
		break;
	case 0x01:
		ret = tim_pro(rec);
		break;
	case 0x02:
		ret = num_pro(rec);
		break;
	case 0x03:
		ret = message_acquire_pro(rec);
		break;
	case 0x04:
		ret = upload_pro(rec);
		break;
	case 0x05:
		ret = download_pro(rec);
		break;
	default:
		break;
	}
	if (ret) {
		rec->code |= 0x80;
		sendREC(pinfo, rec);
	}
	return ret;
}
