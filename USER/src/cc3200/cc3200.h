#ifndef __cc3200_H__
#define __cc3200_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "jd/jdframe.h"
void init_cc3200(void);



typedef struct {
	unsigned char * datestart;
	int datelen;
}PROTOCOL_DAT;


#ifdef __cplusplus
}
#endif

#endif
