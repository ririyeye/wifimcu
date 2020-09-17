#include "wifi/wifi_par.h"

WIFI_ParaTypeDef WIFI_PAR;

const char aimdev[] = { 0x62,0x27,0x21,0x55 };
const char * getaimdev(void)
{	
	return aimdev;
}


WIFI_ParaTypeDef *getWIFI_PAR(void)
{
	return &WIFI_PAR;
}

void wificleanBuffer(WIFI_ParaTypeDef * pesp)
{
	for (int i = 0; i < BUFFER_MAX_INDEX; i++) {
		pesp->buffer[i].flag = 0;
	}
}

databuffer * wifiSearchMethBuffer(WIFI_ParaTypeDef * pesp, search_meth method, void * pri)
{
	for (int i = 0; i < BUFFER_MAX_INDEX; i++) {
		if (method(&pesp->buffer[i], pri)) {
			return &pesp->buffer[i];
		}
	}
	return 0;
}

databuffer * wifiGetBuffer(WIFI_ParaTypeDef * pesp)
{
	for (int i = 0; i < BUFFER_MAX_INDEX; i++) {
		if (0 == pesp->buffer[i].flag)
			return &pesp->buffer[i];
	}
	return 0;
}
