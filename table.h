#ifndef _TABLE_H
#define _TABLE_H

#include "list.h"
#include "tree.h"
#include <string.h>
#include <stdlib.h>
#define table_size 16384
typedef struct Type_ Type;
typedef struct List_ List; 

struct Type_
{
	enum { basic, array, structure, func } kind;
	union
	{
		// 基本类型, 0:int 1:float
		int basic;
		// 数组类型信息包括元素类型与数组大小构成
		struct{ Type* elem; int size; } array;
		// 结构体类型信息是一个链表
		struct{ char *name;/*struct type name "struct ..."*/ struct List_* field; } structure;
		// 函数
		struct {Type* retype; struct List_* param; int define;/*1为定义*/int line;} func;
	} u;
};

struct List_
{
	char* name; 
	Type* type; 
	int level;
	struct list_head list;
	struct list_head collist;
} ;

#define mystrcpy(dest, data); dest=(char *)malloc(strlen(data)+1); strcpy(dest, data);
#define typecpy(dest, data); dest=memcpy(dest, data, sizeof(struct Type_));
#define newlist(data); data=(List *)malloc(sizeof(struct List_));
#define newtype(data); data=(Type *)malloc(sizeof(struct Type_));
#define listcpy(dest, data); dest=(List *)malloc(sizeof(struct List_)); dest=memcpy(dest, data, sizeof(struct List_));
#define structtype(name, data) name=(char *)malloc(sizeof("struct ")+strlen(data)+1); strcpy(name, "struct "); strcat(name, data);
unsigned int hash_pjw(char*);
void init_table();
int sametype(Type*, Type*);
int samekind(int, int);
List* find_table(char*);
int insert_table(char* , Type*); 
void Program(node*);
void ExtDefList(node* );
void ExtDef(node *);
Type* Specifier(node *);
Type* StructSpecifier(node* nd);
void ExtDecList(node *, Type *);
List* VarDec(node *, Type *, int kind); //type为变量类型，kind:0, 2, 3,区分报错类型显示是域还是其他变量
List* DefList(node *nd, int kind); //kind区分域名还是变量名
List* Def(node* nd, int kind);
List* DecList(node *nd, Type *type, int kind);
List* Dec(node *nd, Type* type, int kind);
void FunDec(node *nd, Type* type, int define);
List* VarList(node * nd);
List* ParamDec(node *nd);
void CompSt(node *nd, Type* retype);
void StmtList(node *nd, Type* retype);
void Stmt(node *nd, Type* retype);
Type *Exp(node *nd);
int Args(node *nd, list_head *pa, list_head *ptr);

#endif
