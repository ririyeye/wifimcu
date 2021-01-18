#include "client/client.h"
#include "udpclient/udpclient.h"
#include "cmsis_os2.h"
#include "string.h"
#include "stdio.h"
void testclient(void)
{
	int fd = -1;

	while (fd < 0)
		fd = udpopen_block("45.63.2.213", 9999);

	int num = 100000;
	char buff[32];
	while (1) {
		int len = snprintf(buff, 32, "asdfghjklnum=%d\r\n", num++);
		int sndlen = 0;
		while(1){
			sndlen = updwrite(fd, (unsigned char *)buff, len);

			len = len - sndlen;
			if (0 != len) {
				if (sndlen != 0) {
					memmove(buff, buff + sndlen, len);
				}
				osDelay(1);
				continue;
			}
			break;
		}
	}
}
