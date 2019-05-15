#include "mip32.h"
#include <string.h>
#include <stdio.h>

extern InterCode INTERCODES;
FILE *outs;
int addr[0x3fff];
int regnow=0, addrnow=0, paramnow = 0, argnow = 0;

void gen(char * s) {
	fprintf(outs, "%s", s);
}

int offset(char * id) {
	if(addr[hash_pjw(id)] == 0){  
		addrnow += 4;
		addr[hash_pjw(id)] = -addrnow;
		gen("addi $sp, $sp, -4\n");
	}
	return addr[hash_pjw(id)];
}

char *reg(char *id) {
	char * s = malloc(32);
	sprintf(s, "$t%d", regnow);
	regnow = (regnow + 1) % 8;
	if(strlen(id) == 0) return s;
	if(id[0] == '*') {
		fprintf(outs, "lw %s, %d($fp)\n", s, offset(&id[1]));
		fprintf(outs, "lw %s, 0(%s)\n", s, s);
	} else if(id[0] == '&') {
		fprintf(outs, "lw %s, %d($fp)\n", s, offset(&id[1]));
	} else {
		fprintf(outs, "lw %s, %d($fp)\n", s, offset(id));
	}
	return s;
}

void init_asm(){
	fprintf(outs, ".data\n");
	fprintf(outs, "_prompt: .asciiz \"Enter an interger:\"\n");
	fprintf(outs, "_ret: .asciiz \"\\n\"\n");
	fprintf(outs, ".globl main\n");
	fprintf(outs, ".text\n");
	fprintf(outs, "read:\n"
				" li $v0, 4\n"
				" la $a0, _prompt\n"
				" syscall\n"
				" li $v0, 5\n"
				" syscall\n"
				" jr $ra\n");
	fprintf(outs, "write:\n"
				" li $v0, 1\n"
				" syscall\n"
				" li $v0, 4\n"
				" la $a0, _ret\n"
				" syscall\n"
				" move $v0, $0\n"
				" jr $ra\n");
}

void Translate_asm() {
	init_asm();
	list_head *p1;
	list_foreach(p1, &INTERCODES.iclist) {
		InterCode *target=list_entry(p1, InterCode, iclist);
		char *ict=target->val;
		int iclen=strlen(ict);
		int ictkind=target->kind;

		char *op[6];
		int cnt=0, i;
		op[0]=ict;
		for(i=0; ict[i]!='\0'; i++) {
			if (ict[i]==' ') {
				ict[i]='\0';
				op[++cnt]=&ict[i+1];
			}
		}

		char *r1;
		switch(ictkind) {
			case(LABELIC):
				gen(mystrcat(2, op[1], ":\n"));
				break;
			case(FUNCTIONIC) :
				addrnow = 0;
				paramnow = 0;
				gen(mystrcat(2, op[1], ":\n"));
				gen("addi $sp, $sp, -4\n"
							"sw $fp, 0($sp)\n"
							"move $fp, $sp\n");
				break;
			case(ASSIGNIC) :
				r1 = reg(op[0]);
				if(op[2][0] != '#') {
					gen(mystrcat(5, "move ", r1, ", ", reg(op[2]), "\n"));
				} else {
					gen(mystrcat(5, "li ", r1, ", ", &op[2][1], "\n"));
				}
				storeaddr(r1, op[0]);
				break;
			case(PLUSIC) :
				r1 = reg(op[0]);
				gen(mystrcat(7, "add ", r1, ", ", reg(op[2]), ", ", reg(op[4]), "\n"));
				storeaddr(r1, op[0]);
				break;
			case(MINUSIC) :
				r1 = reg(op[0]);
				gen(mystrcat(7, "sub ", r1, ", ", reg(op[2]), ", ", reg(op[4]), "\n"));
				storeaddr(r1, op[0]);
				break;
			case(STARIC) :
				r1 = reg(op[0]);
				if(op[4][0] != '#') {
					gen(mystrcat(7, "mul ", r1, ", ", reg(op[2]), ", ", reg(op[4]), "\n"));
				} else {
					gen(mystrcat(7, "mul ", r1, ", ", reg(op[2]), ", ", &op[4][1], "\n"));
				}
				storeaddr(r1, op[0]);
				break;
			case(DIVIC) :
				r1 = reg(op[0]);
				gen(mystrcat(5, "div ", reg(op[2]), ", ", reg(op[4]), "\n"));
				gen(mystrcat(3, "mflo ", r1, "\n"));
				storeaddr(r1, op[0]);
				break;
			case(GOTOIC):
				gen(mystrcat(3, "j ", op[1], "\n"));
				break;
			case(IFIC):
				if(op[2][0] == '=') gen(mystrcat(7, "beq ", reg(op[1]), ", ", reg(op[3]), ", ", op[5], "\n"));
				else if(op[2][0] == '!') gen(mystrcat(7, "bne ", reg(op[1]), ", ", reg(op[3]), ", ", op[5], "\n"));
				else if(op[2][0] == '>') {
					if(strlen(op[2]) == 1) gen(mystrcat(7, "bgt ", reg(op[1]), ", ", reg(op[3]), ", ", op[5], "\n"));
					else gen(mystrcat(7, "bge ", reg(op[1]), ", ", reg(op[3]), ", ", op[5], "\n"));
				} else {
					if(strlen(op[2]) == 1) gen(mystrcat(7, "blt ", reg(op[1]), ", ", reg(op[3]), ", ", op[5], "\n"));
					else gen(mystrcat(7, "ble ", reg(op[1]), ", ", reg(op[3]), ", ", op[5], "\n"));
				}
				break;
			case(RETURNIC):
				gen(mystrcat(3, "move $v0, ", reg(op[1]), "\n"));
				gen("move $sp, $fp\n"
							"lw $fp, 0($sp)\n"
							"addi $sp, $sp, 4\n"
							"jr $ra\n");
				break;
			case(DECIC):
				r1 = reg("");
				addrnow += atoi(op[2]);
				char saddr[32];
				sprintf(saddr, "%d", atoi(op[2]) + 4);
				gen(mystrcat(3, "addi $sp, $sp, -", saddr,"\n"));
				sprintf(saddr, "%d", addrnow);
				gen(mystrcat(5, "addi ", r1, ", $fp, -", saddr, "\n"));
				addrnow += 4;
				sprintf(saddr, "%d", addrnow);
				gen(mystrcat(5, "sw ", r1, ", -", saddr, "($fp)\n"));
				addr[hash_pjw(op[1])] = -addrnow;
				break;
			case(ARGIC):
				gen("addi $sp, $sp, -4\n");
				gen(mystrcat(3, "sw ", reg(op[1]), ", 0($sp)\n"));
				argnow ++;
				break;
			case(CALLIC):
				gen(mystrcat(3, "addi $sp, $sp, -4\n"
							"sw $ra, 0($sp)\n"
							"jal ", op[3], "\n" 
							"lw $ra, 0($sp)\n"
							"addi $sp, $sp, 4\n"));
				char sint[32];
				sprintf(sint, "%d", 4 * argnow);
				gen(mystrcat(3, "addi $sp, $sp, ", sint, "\n"));

				gen(mystrcat(3, "move ", reg(op[0]), ", $v0\n"));
				storeaddr(r1, op[0]);
				argnow = 0;
				break;
			case(PARAMIC):
				paramnow ++;
				addr[hash_pjw(op[1])] = 8 + 4 * (paramnow-1);
				break;
			case(READIC):

				r1 = reg(op[1]);

				gen(mystrcat(3, "addi $sp, $sp, -4\n"
								"sw $ra, 0($sp)\n"
								"jal read\n"
								"lw $ra, 0($sp)\n"
								"addi $sp, $sp, 4\n"
								"move ", r1, ", $v0\n"));
				storeaddr(r1, op[1]);

				break;
			case(WRITEIC):

				gen(mystrcat(3, "move $a0, ",reg(op[1]),"\n"
								"addi $sp, $sp, -4\n"
								"sw $ra, 0($sp)\n"
								"jal write\n"
								"lw $ra, 0($sp)\n"
								"addi $sp, $sp, 4\n"));
				break;
		}

	}
}
