#include "table.h"
#include "list.h"
#include <stdio.h>

List table[table_size];
List table2[table_size];
int level;
unsigned int hash_pjw(char* name)
{
	unsigned int val = 0, i;
	for (; *name; ++name)
	{
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

void init_table() {
	int i;
	for (i=0; i<table_size; i++) {
		list_init(&table[i].list);
		list_init(&table2[i].collist);
	}
}

int sametype(Type* type1, Type* type2) { //不可能出现函数类型，函数调用时参数填return type
	//printf("s sametype\n");
	int typ1=type1->kind, typ2=type2->kind;
	if (typ1!=typ2) return 0;
	if (typ1==0) {
		//printf("!!!\n");
		if (type1->u.basic!=type2->u.basic) return 0;
	}

	else if (typ1==1) {
		if (type1->u.array.elem==NULL) {
			//printf("1的elem空\n");
			return 0;
		} 
		if (type2->u.array.elem==NULL) {
			//printf("2 elem NULL\n");
			return 0;
		}
		if (type1->u.array.size!=type2->u.array.size) {
			//printf("not same array size\n");
			return 0;
		}
		if (sametype(type1->u.array.elem, type2->u.array.elem)==0) {
			//printf("0 end sametype\n"); return 0;
		}
		else {
			//printf("1 end sametype 1\n"); return 1;
		}
	}	

	else {
/*		if (type1->u.structure.name==NULL) {
			printf("1 struct without name end sametype\n");
			return 0;
		}
		if (type2->u.structure.name==NULL) {
			printf(" 2 struct without name end sametype\n");
			return 0;
		}
*/		list_head *p1;
		list_head*p2=&type2->u.structure.field->list;
		list_foreach(p1, &type1->u.structure.field->list){
			p2=p2->next;
			if(p2==&type2->u.structure.field->list) {
				return 0;
			}
			Type* typ1 = list_entry(p1,List,list)->type;
			Type* typ2 = list_entry(p2,List,list)->type;
			if(!sametype(typ1,typ2)) {
				return 0;
			}

		}
		return p2->next == &type2->u.structure.field->list;
/*		
		if (strcmp(type1->u.structure.name, type2->u.structure.name)==0) {
			printf("1 end sametype\n"); return 1;
		}
		else {
			printf("0 end sametype\n"); return 0;
		}
*/
	}	
	//printf("1 end sametype e\n");
	return 1;
}	

int samefunc(Type* func1, Type* func2) {//判断func是否一样
	if(!sametype(func1->u.func.retype, func2->u.func.retype))return 0;
	list_head *p1;
	list_head*p2=&func2->u.func.param->list;
	list_foreach(p1, &func1->u.func.param->list){
		p2=p2->next;
		if(p2==&func2->u.func.param->list) {
			return 0;
		}
		Type* type1=list_entry(p1,List,list)->type;
		Type* type2=list_entry(p2,List,list)->type;
		if(!sametype(type1,type2)) {
			return 0;
		}

	}
	return p2->next == &func2->u.func.param->list;

}

List* find_table(char* name) { //samename&kind:found
	if (name==NULL) { //printf("in find_table:name=NULL\n");  
		return NULL; }
//	if (type==NULL) { printf("in find_table:name=NULL\n"); return NULL; }
//printf("to find %s\n", name);
	unsigned int hash=hash_pjw(name);
	if (list_empty(&table[hash].list)) {
		//printf("in find_table:hash entry is NULL\n");
		return NULL; //not found
	}
	struct list_head* pos=&table[hash].list;
	while (pos->next!=&table[hash].list) {
		pos=pos->next;
		List *entry; newlist(entry);
		entry=list_entry(pos, struct List_, list);
		if (strcmp(entry->name, name)==0 /*&& samekind(type->kind, entry->type->kind)*/) {
//printf("success find entry %s\n", name);	
			return entry;
		}
	}
	//printf("in find_table:hash entry is not NULL but no this name\n");
	return NULL;
}	

int insert_table(char* name, Type* type) {
//	if (find_table(table, type)) return;
	if (name==NULL || type==NULL) {//printf("insert table failed , name||type=null\n"); 
		return 0;}
	int hash=hash_pjw(name);
	List *data; newlist(data);
	mystrcpy(data->name, name); //data->type=type;
	newtype(data->type);
	memcpy(data->type, type, sizeof(struct Type_));
	list_add_after(&table[hash].list, &data->list);
	list_add_after(&table2[level].collist, &data->collist);
	//printf("insert success %s\n", name);
	return 1;
}

void Program(node* nd) {
	//printf("s program\n");
/*	Type *readtype; newtype(readtype);
	readtype->kind=3; insert_table("read", readtype);
	Type *writetype; newtype(writetype);
	writetype->kind=3; insert_table("write", writetype);
*/
	ExtDefList(nd->chi[0]);
	list_head* ptr;
	//printf("%d\n", level);
	list_foreach(ptr, &table2[level].collist){
		List* sym=list_entry(ptr, List, collist);
		if(sym->type->kind==3 && sym->type->u.func.define==0) {
			printf("Error type 18 at line %d:\n", sym->type->u.func.line);
		}
	}
}

//global def list
void ExtDefList(node* nd) {
	//printf("s extdeflist\n");
	if (nd->cn==0) return;
	//printf("m extdeflist\n");
	ExtDef(nd->chi[0]);
	ExtDefList(nd->chi[1]);
}

//global def of var/struct/func
void ExtDef(node * nd) {
	//printf("s extdef\n");
	Type* type=Specifier(nd->chi[0]);
//printf("have re specifier\n");
	if (strcmp(nd->chi[1]->name, "SEMI")==0) return;
	else if (strcmp(nd->chi[1]->name, "ExtDecList")==0) {
		ExtDecList(nd->chi[1], type);//inherent!!!
		return;
	}
	else {
		if(nd->chi[2]->name[0]=='C') {
			FunDec(nd->chi[1], type, 1);//type:func return_type
			CompSt(nd->chi[2], type);//return type;
		} else {
			FunDec(nd->chi[1], type, 0);//type:func return_type
			
		}
	}	
}	

Type* Specifier(node *nd) {
	//printf("s specifier\n");
	Type* type; newtype(type);//分配出一块新内存，存放该节点的type
	if (strcmp(nd->chi[0]->name, "TYPE")==0) {
		type->kind=0;
//printf("val:%s\n", nd->chi[0]->val);
		if (strcmp(nd->chi[0]->val, "int")==0) {
			type->u.basic=0; 
		}
		else if (strcmp(nd->chi[0]->val, "float")==0)
		    type->u.basic=1;
		else ;//printf("not TYPE!!BUG!\n");
	}
	else type=StructSpecifier(nd->chi[0]);
	return type;
}

Type* StructSpecifier(node* nd) {
//printf("s structspecifier\n");
	Type* type=malloc(sizeof(struct Type_));
		
	if (strcmp(nd->chi[1]->name, "Tag")==0) {
		node *tagnd=nd->chi[1];
		List* res; newlist(res); 
		char *name; structtype(name, tagnd->chi[0]->val);	
		List *find=find_table(name);//name="struct ..."
//printf("have found\n");
		
		if (find==NULL || find->type->kind!=2 || strcmp(find->name, find->type->u.structure.name)!=0) {//name equiv
			printf("Error type 17 at line %d: Undefined struct type\n", tagnd->par->line);
			return NULL;
		}	

		listcpy(res, find);
		return res->type;
	}

	else {
		node* optnd=nd->chi[1];
		type->kind=2;
		if (optnd->cn==0) type->u.structure.name=NULL;//struct without name
		else {
			structtype(type->u.structure.name, optnd->chi[0]->val);
			List *res=find_table(type->u.structure.name);
			if (res!=NULL && res->level==level && res->type->kind==2 && strcmp(res->name, res->type->u.structure.name)==0) {
				printf("Error type 16 at line %d: struct type name has been defined\n", optnd->line);
				return NULL;
			}
		}
	    type->u.structure.field=DefList(nd->chi[3], 2);//域名
		if(type->u.structure.name != NULL)insert_table(type->u.structure.name, type);//"struct ..."
		return type;
	}	
}	

void ExtDecList(node *nd, Type *type) {
//printf("s extdeclist\n");
	if (nd==NULL || type==NULL) return ;
	VarDec(nd->chi[0], type, 0);//basic,array
	if (nd->cn>1) ExtDecList(nd->chi[2], type);
}

List* VarDec(node *nd, Type *type, int kind) { //type为变量类型，kind:0, 2, 3,区分报错类型显示是域还是其他变量
//printf("s vardec type \n");
	if (type==NULL) return NULL;
	List* ans; newlist(ans);
//	list_init(&(res->list));
//printf("%s\n",nd->chi[0]->name);
	if (strcmp(nd->chi[0]->name, "ID")==0) {
		node *idnd=nd->chi[0];
	//	List* ans; newlist(ans);
		mystrcpy(ans->name, idnd->val);
		newtype(ans->type); //ans->type=type;
		typecpy(ans->type, type);
		List* tmpf=find_table(idnd->val);
		if (tmpf!=NULL && tmpf->level == level) { //have defined
			if (kind==2) printf("Error type 15 at line %d: Field name has been defined\n",idnd->par->line);
			else printf("Error type 3 at line %d: Var or array name has been defined\n", idnd->par->line);
			return NULL;
		}	
		if (strcmp(nd->par->name, "VarDec")!=0) {
			insert_table(idnd->val, type);
			//if(type->kind == 2 && type->u.structure.field == NULL) printf("** **\n");
		}
		return ans;
	}

	else {
		List* front; newlist(front);
		front=VarDec(nd->chi[0], type, 1);	
		if (front == NULL) return NULL; //之前的部分已经出错
		Type* tmp; //newtype(tmp);
	//	typecpy(tmp, front->type);
		tmp=front->type;
		while(tmp->kind==1) {
			tmp=tmp->u.array.elem;
		}
		tmp->kind=1;
		newtype(tmp->u.array.elem);
		typecpy(tmp->u.array.elem, type); 
		tmp->u.array.size=atoi(nd->chi[2]->val);
	//	front->type=tmp;
		List* tmpf=find_table(front->name);
		if (tmpf!=NULL && tmpf->level ==level) { //have defined
			if (kind==2) printf("Error type 15 at line %d: Field name has been defined\n",nd->par->line);
			else printf("Error type 3 at line %d: Var or array name has been defined\n", nd->par->line);
			return NULL;
		}	
		if (strcmp(nd->par->name, "VarDec")!=0) insert_table(front->name, front->type);
		return front;
	}	
}	//return list_data

List* DefList(node *nd, int kind) { //kind区分域名还是变量名
//printf("s deflist\n");
	if (nd->cn==0) return NULL;
	List* ans=malloc(sizeof(struct List_));
	list_init(&ans->list);
	List *temp=Def(nd->chi[0], kind);
	if(temp!=NULL) list_merge_before(&ans->list, &temp->list);
//	else return NULL;
	List* tmp=DefList(nd->chi[1], kind);
	if (tmp!=NULL) list_merge_before(&ans->list, &tmp->list);//def before deflist
	return ans;//return list_head
}	

List* Def(node* nd, int kind) {
//printf("s def\n");
	Type* type=Specifier(nd->chi[0]);
	List* ans=DecList(nd->chi[1], type, kind);
	return ans;
} //return list_head

List* DecList(node *nd, Type* type, int kind) {
//printf("s declist\n");
	if (type==NULL) return NULL;
	List* ans; newlist(ans);
	list_init(&ans->list);
	List *temp=Dec(nd->chi[0], type, kind);
	if (temp!=NULL) list_merge_before(&ans->list, &temp->list);
	if (nd->cn>1) {
		List* tmp=DecList(nd->chi[2], type, kind);
		if (tmp!=NULL) list_merge_before(&ans->list, &tmp->list);
	}
	return ans;
} //return list_head

List* Dec(node *nd, Type* type, int kind) {
//printf("s dec\n");
	if (type==NULL) return NULL;
	List* ans; newlist(ans); list_init(&ans->list);
	List *tmp=VarDec(nd->chi[0], type, kind);
	if (tmp!=NULL) list_add_before(&ans->list,&tmp->list);
	if (nd->cn==1) return ans;
	if (kind==2) {
		printf("Error type 15 at line %d: field can't be initialized\n", nd->chi[0]->line);
		return ans;
	}
	Type* e=Exp(nd->chi[2]);
	if (e!=NULL && sametype(e, type)==0) {
		printf("Error type 5 at line %d: not same type of assignop\n", nd->chi[0]->line);
	}
	return ans;
}	//return list_head

void  FunDec(node *nd, Type* type, int define) {
//printf("s fundec\n");
	level ++;
	node* idnd=nd->chi[0];
	List* fun; newlist(fun);
	mystrcpy(fun->name, idnd->val);
	newtype(fun->type);
	fun->type->kind=3;
	newtype(fun->type->u.func.retype);
	typecpy(fun->type->u.func.retype, type);
	newlist(fun->type->u.func.param); 
	list_init(&fun->type->u.func.param->list);
	if (nd->cn>3) {
		List *tmp=VarList(nd->chi[2]);
		if (tmp!=NULL) fun->type->u.func.param=tmp;
	}
	level --;
	List* findf=find_table(idnd->val);
	if (findf!=NULL) {
		if(findf->type->u.func.define && define) {
			printf("Error type 4 at line %d: function name has been defined\n", idnd->par->line);
			return;
		}
		if(!samefunc(findf->type, fun->type)){
			printf("Error type 19 at line %d:\n", idnd->par->line);
			return;
		}
		findf->type->u.func.define |= define;
		return;
	}
	fun->type->u.func.define = define;
	fun->type->u.func.line=nd->line;
	insert_table(idnd->val, fun->type);
}

List* VarList(node * nd) {
//printf("s varlist\n");
	List* p=malloc(sizeof(struct List_));
	list_init(&p->list);
	List* pa=ParamDec(nd->chi[0]);
	if(pa!=NULL) list_merge_before(&p->list, &pa->list);
	if (nd->cn>1) {
		List* tmp=VarList(nd->chi[2]);
		if(tmp!=NULL) list_merge_before(&p->list, &tmp->list);
	}
	return p;
} //return list_head

List *ParamDec(node *nd) {
//printf("s paramdec\n");
	Type *type=Specifier(nd->chi[0]);
	List *ans; newlist(ans); 
	list_init(&ans->list);
	List *data=VarDec(nd->chi[1], type, 3);
	if (data!=NULL) list_add_before(&ans->list, &data->list);
	return ans;
} //return list_head

void CompSt(node *nd, Type* retype) {
//printf("s compst\n");
	level++;
	DefList(nd->chi[1], 0);
	StmtList(nd->chi[2], retype);
	level--;
	return;
	List *ptr;
	while(!list_empty(&table2[level].collist)) {
		ptr = list_entry(table2[level].collist.next, List, collist);
		list_del(&ptr->list);
		list_del(&ptr->collist);
	}
	
}

void StmtList(node *nd, Type* retype) {
//printf("s stmtlist\n");
	if (nd->cn==0) return;
	Stmt(nd->chi[0], retype);
	StmtList(nd->chi[1], retype);
}

void Stmt(node *nd, Type* retype) {
//printf("s stmt\n");
	if(nd->cn==2) {
		Type *type = Exp(nd->chi[0]);
	}
	else if(nd->cn == 1) {
		CompSt(nd->chi[0], retype);
	}
	else if(nd->cn==3) {
		Type *type = Exp(nd->chi[1]);
		if(type == NULL)return;
		if(!sametype(type, retype)) {
			printf("Error type 8 at line %d :\n", nd->line);
			return ;
		}
	}
	else if(nd->cn == 5) {
		Type *type=Exp(nd->chi[2]);
		Stmt(nd->chi[4], retype);
		if(type == NULL) return;
		if(!(type->kind==0&&type->u.basic==0)) {
			printf("Error type 7 at line %d :\n", nd->line);
			return ;
		}
	}
	else if(nd->cn == 7) {
		Type *type=Exp(nd->chi[2]);
		Stmt(nd->chi[4], retype);
		Stmt(nd->chi[6], retype);
		if(type == NULL) return;
		if(!(type->kind==0&&type->u.basic==0)) {
			printf("Error type 7 at line %d :\n", nd->line);
			return ;
		}
	
	}
}

Type* Exp(node *nd) {
//printf("s exp\n");
	Type* ans; newtype(ans);

	if (nd->cn==3) {
		if (strcmp(nd->chi[1]->name, "ASSIGNOP")==0) {
			node *left=nd->chi[0];
			if (!( strcmp("ID", left->chi[0]->name)==0 || ( strcmp("Exp", left->chi[0]->name)==0 && strcmp("LB", left->chi[1]->name)==0 && strcmp("Exp", left->chi[2]->name)==0 && strcmp("RB", left->chi[3]->name)==0 ) || ( strcmp("Exp", left->chi[0]->name)==0 && strcmp("DOT", left->chi[1]->name)==0 && strcmp("ID", left->chi[2]->name)==0 ) )) {
				printf("Error type 6 at line %d : assignop left\n", left->line);
				return NULL;
			}
			Type* t1=Exp(nd->chi[0]);
			Type* t2=Exp(nd->chi[2]);
			if(t1 == NULL || t2 == NULL) return NULL;
			if (sametype(t1, t2)==0) {
				printf("Error type 5 at line %d : assign op not match\n", left->line);
				return NULL;
			}
			return t1;
		}

		else if (strcmp(nd->chi[1]->name, "AND")==0 || strcmp(nd->chi[1]->name, "OR")==0 ) {
			Type* t1=Exp(nd->chi[0]);
			Type* t2=Exp(nd->chi[2]);
			if(t1 == NULL || t2 == NULL) return NULL;
			if (sametype(t1, t2)==0) {
				printf("Error type 7 at line %d: not match\n", nd->chi[0]->line);
				return NULL;
			}
			return t1;
		}

		else if (strcmp(nd->chi[1]->name, "RELOP")==0) {
			Type* t1=Exp(nd->chi[0]);
			Type* t2=Exp(nd->chi[2]);
			if(t1 == NULL || t2 == NULL) return NULL;
			if (sametype(t1, t2)==0) {
				printf("Error type 7 at line %d: not match\n", nd->chi[0]->line);
				return NULL;
			}
			ans->kind=0; ans->u.basic=0;
			return ans;
		}	

		else if (strcmp(nd->chi[1]->name, "PLUS")==0 || strcmp(nd->chi[1]->name, "MINUS")==0 || strcmp(nd->chi[1]->name, "STAR")==0 || strcmp(nd->chi[1]->name, "DIV")==0) {
			Type* t1=Exp(nd->chi[0]);
			Type* t2=Exp(nd->chi[2]);
			if(t1 == NULL || t2 == NULL) return NULL;
			if (sametype(t1, t2)==0 ||t1->kind!=0 ||t2->kind!=0) {
				printf("Error type 7 at line %d: not match\n", nd->chi[0]->line);
				return NULL;
			}
			return t1;
		}

		else if (strcmp(nd->chi[1]->name, "Exp")==0) {
			return Exp(nd->chi[1]);
		}

		else if (strcmp(nd->chi[1]->name, "LP")==0) {
			node *idnd=nd->chi[0];
			List *res=find_table(idnd->val); 
			if (res==NULL) {
//				printf("Error type 2 at line %d: func undefined\n", nd->line);
				return NULL;
			}
			if (res->type->kind!=3) {
				printf("Error type 11 at line %d: this id is not func\n", nd->line);
				return NULL;
			}
			return res->type->u.func.retype;
		}

		else { //struct field access
			node *expnd=nd->chi[0];
			Type *exp=Exp(expnd);
			if (exp==NULL) {
				printf("Error type 1 at line %d: struct undefined\n", nd->line);
				return NULL;
			}
			if (exp->kind!=2) {
				printf("Error type 13 at line %d: not struct\n", nd->line);
				return NULL;
			}
			node *idnd=nd->chi[2];
			list_head* ptr;
//			if(exp->u.structure.field == NULL) printf("!!!\n");
			list_foreach(ptr, &exp->u.structure.field->list) {
				List *tmp=list_entry(ptr, struct List_, list);
				//printf("%s %s\n", tmp->name, idnd->val);
				if (strcmp(tmp->name, idnd->val)==0) return tmp->type;
			}
			printf("Error type 14 at line %d: field not exist\n", nd->line);
			return NULL;
		}
	}

	else if (nd->cn==4) {
		if (strcmp(nd->chi[0]->name, "ID")==0) {
			node *idnd=nd->chi[0];
			List *res=find_table(idnd->val); 
			if (res==NULL) {
//				printf("Error type 2 at line %d: func undefined\n", nd->line);
				return NULL;
			}
			if (res->type->kind!=3) {
				printf("Error type 11 at line %d: this id is not func\n", nd->line);
				return NULL;
			}
			int temp=Args(nd->chi[2], &res->type->u.func.param->list, res->type->u.func.param->list.next);
			if (temp==0) {
				printf("Error type 9 at line %d: \n", nd->line);
				return NULL;
			}
			return res->type->u.func.retype;
		}
		else {
			Type *type1 = Exp(nd->chi[0]);
			Type *type2 = Exp(nd->chi[2]);
			if(type1 == NULL || type2 == NULL) return NULL;
			if(type1->kind != 1) {
				printf("Error type 10 at line %d: \n", nd->line);
				return NULL;
			}
			if(type2->kind!=0 || (type2->kind==0 && type2->u.basic!=0)) {
				printf("Error type 12 at line %d:\n", nd->line);
				return NULL;
			}
			return type1->u.array.elem;
		}
	} 

	else if (nd->cn == 2) {
		Type *type=Exp(nd->chi[1]);
		if(type==NULL) return NULL;
		if(strcmp(nd->chi[0]->name, "MINUS")==0) {
			if(type->kind!=0){
				printf("Error type 7 at line %d:\n", nd->line);
				return NULL;
			}
			return type;
		} 
		else {
			if(!(type->kind==0 && type->u.basic==0)){
				printf("Error type 7 at line %d:\n", nd->line);
				return NULL;
			}
			return type;
		}
	}

	else if (nd->cn == 1) {
		if(strcmp(nd->chi[0]->name, "ID")==0) {
			List* entry= find_table(nd->chi[0]->val);
			if(entry == NULL){
				printf("Error type 1 at line %d:\n", nd->line);
				return NULL;
			}
			return entry->type;
		} 
		else if (strcmp(nd->chi[0]->name, "INT")==0) {
			Type *type;
			newtype(type);
			type->kind = 0;
			type->u.basic=0;
			return type;
		}
		else {
			Type *type;
			newtype(type);
			type->kind = 0;
			type->u.basic=1;
			return type;
	
		}
	}

//	return ans;
}	

int Args(node *nd, list_head* head, list_head * ptr) {
	//printf("s args\n");
	if(ptr == head) return 0;
	List * arg=list_entry(ptr, List, list);
	Type *type = Exp(nd->chi[0]);
	if(type == NULL){
		return 1;
	}
	if(!sametype(type, arg->type))return 0;
	if(nd->cn==3)return Args(nd->chi[2], head, ptr->next);
	return ptr->next==head;
}
