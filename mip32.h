#ifndef _MIP32_H
#define _MIP32_H

#include "intercode.h"

#define storeaddr(r,id); if(id[0] == '*') { char * rr = reg(""); fprintf(outs, "lw %s, %d($fp)\n", rr, offset(&id[1])); \
		fprintf(outs, "sw %s, 0(%s)\n", r, rr); } else fprintf(outs, "sw %s, %d($fp)\n", r, offset(id)); 

void Translate_asm();
char *reg(char *);


#endif
