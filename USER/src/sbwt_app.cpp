
#include "sbwt.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

//#include <arm_acle.h>

int a48decode(TPack *pack, SBWT_INFO &session)
{
	int32_t p4data;
	int16_t p2data;
	if (!pack) {
		return SBWT_INFO::sta_error_pack;
	}

	if (pack->sz < 7) {
		printf("sz error\n");
		return SBWT_INFO::sta_error_pack;
	}

	if (pack->buff[0] & (1 << 1)) {
		printf("removed\n");
		return SBWT_INFO::sta_error_pack;
	}
#if 0
	if (pack->buff[0] & (0B11 << 6))
#else
	if (pack->buff[0] & (3 << 6))
#endif
	{
		//printf("11 find\n");
#if 1
		int32_t tmp32;
		memcpy(&tmp32, &pack->buff[1], 4);
		p4data = __rev(tmp32);

		int16_t tmp16;
		memcpy(&tmp16, &pack->buff[5], 2);
		p2data = __rev(tmp16) >> 16;
#else
		memcpy(&p4data, &pack->buff[1], 4);
		memcpy(&p2data, &pack->buff[5], 2);
#endif
		session.SetData(p4data, p2data);
	}
	return SBWT_INFO::sta_ok;
}

int errRet(TPack *pack, SBWT_INFO &session)
{
	if (!pack) {
		return SBWT_INFO::sta_error_pack;
	}

	if (pack->sz > 0) {
		printf("error = %d\n", pack->buff[0]);

		return pack->buff[0];
	}

	return SBWT_INFO::sta_unknuow;
}

const APPDECODE DataCodeGroup[] = {
	{ 0x30, a48decode },
	{ 0x23, errRet },
};

int checkAppData(TPack *pack, SBWT_INFO &session)
{
	if (!pack) {
		return -2;
	}

	for (auto cod : DataCodeGroup) {
		if (cod.code == pack->code) {
			return cod.funa(pack, session);
		}
	}
	return -1;
}
