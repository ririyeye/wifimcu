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

	int num = 0;
	char buff[32];
	while (1) {
		int len = snprintf(buff, 32, "num=%d\r\n", num++);
		updwrite(fd, (unsigned char *)buff, len);
		osDelay(3);
	}
}
