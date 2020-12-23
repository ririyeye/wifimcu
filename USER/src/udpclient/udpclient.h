#ifndef __UDPCLIENT_H____
#define __UDPCLIENT_H____

int udpopen_block(const char * addr,int port);
int udpread(int fd,unsigned char * dat,int maxlen);
int updwrite(int fd,unsigned char * dat,int len);
int udpclose_block(int fd);

//upd init
enum udpstatus {
	updconnected = 0,
	udpclosed = -1,
	udpnofd = -2,
	udpoperator_failed = -3,

	udp_par_error = -100,
};

struct UDPFD {
	int fd;
	int connectID;
	udpstatus nowstatus;
	udpstatus cmdstatus;
#define SERVERLEN 32
	char servername[SERVERLEN];
	int remoteport;
#define RXMAX 1024
	unsigned char rxbuf[RXMAX];
	int rxend;
	int rxMax;
#define TXMAX 1024
	unsigned char txbuf[TXMAX];
	int txend;
	int txMax;
};

void udpinit(void);
UDPFD * udpout(void);
int udpin(int fd ,unsigned char * dat, int len);


typedef int UDPFDitr;
void UDPFDitr_init(UDPFDitr *itr);
int UDPFDitr_getNext(UDPFDitr *itr);
UDPFD *UDPFD_Get(UDPFDitr *itr);
void udpfd_set(UDPFD *fd, int status);
#endif
