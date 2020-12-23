#include "client/client.h"
#include "udpclient/udpclient.h"
#include "cmsis_os2.h"

void testclient(void)
{
	int fd = -1;

	while (fd < 0)
		fd = udpopen_block("45.63.2.213", 9999);

	while (1) {
		updwrite(fd, (unsigned char *)"test\r\n", 6);
		osDelay(1000);
	}
}
