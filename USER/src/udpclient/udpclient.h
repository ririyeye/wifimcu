#ifndef __UDPCLIENT_H____
#define __UDPCLIENT_H____

int udpopen(const char * addr,int port);
int udpread(int fd,unsigned char * dat,int len);
int updwrite(int fd,unsigned char * dat,int maxlen);
int udpclose(int fd);

//upd init
enum udpstatus {
	updopen = 0,
	udpclosed = -1,
};

struct UDPFD {
	int fd;
	int connectID;
	udpstatus nowstatus;
	udpstatus cmdstatus;
	unsigned char rxbuf[1024];
	int rxinit, rxend;
	unsigned char txbuf[1024];
	int txinit, txend;
};

void udpinit(void);
UDPFD * udpout(void);
int udpin(int fd ,unsigned char * dat, int len);
#endif
