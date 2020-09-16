#ifndef __ebyte_lora_H____
#define __ebyte_lora_H____

#include "myuart.h"
#include "sbwt.h"
void init_ebyte_lora();
SBWT_INFO *GetSbwt_lora(int index);

struct ebyte_operator {
	int init();
	void Lora_MD0_write(int val);
	void Lora_MD1_write(int val);
};

#endif
