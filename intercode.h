#ifndef _INTERCODE_H
#define _INTERCODE_H

#include "table.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct Operand_ Operand;
typedef struct InterCode_ InterCode;

//operand kind , ID includes function, variable...
enum { IDOP, CONSTANTOP, ADDRESSOP, LABELOP };

//intercode kind
enum { LABELIC, FUNCTIONIC, ASSIGNIC, PLUSIC, MINUSIC, STARIC, DIVIC, IFIC, GOTOIC, RETURNIC, DECIC, ARGIC, CALLIC, PARAMIC, READIC, WRITEIC }; 

struct Operand_ {
	Type *type;
	char *val;
	struct list_head list;
};

struct InterCode_ {
	int kind;
	char *val; //eg:LABEL label1:    x:=y    ...仅当前的这一句
	struct list_head iclist;
	struct list_head oplist;
};

#define newic(data); data=(InterCode *)malloc(sizeof(struct InterCode_));
#define newop(data); data=(Operand *)malloc(sizeof(struct Operand_));
#define recordop(tmp, data); list_init(&tmp->oplist); Operand* top; newop(top); mystrcpy(top->val, data); list_add_before(&tmp->oplist, &top->list);

char* mystrcat(int, ...);
char *newt();
char *newlabel();
char *lastt();
void print_intercodes();
void Translate_Program(node *nd);
void Translate_ExtDefList(node *nd);
void Translate_ExtDef(node * nd);
void Translate_FunDec(node *nd);
void Translate_VarList(node* nd);
void Translate_ParamDec(node* nd);
char *Translate_VarDec(node *nd, int kind);
void Translate_CompSt(node *nd);
void Translate_DefList(node *nd);
void Translate_DecList(node *nd);
void Translate_Dec(node *nd);
char *Translate_Exp(node *nd);
void Translate_StmtList(node *nd);
void Translate_Stmt(node*);
void Translate_Cond(node *, char *, char *);
void Translate_Args(node *, InterCode *);

#endif
