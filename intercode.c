#include "intercode.h"
#include <stdio.h>
#include <string.h>

InterCode INTERCODES;
int varno, labelno;
int isparam[0x3fff];

char *mystrcat(int carg, ...) {
	va_list ap;
	va_start(ap, carg);
	int i, len=1;
	for (i=0; i<carg; i++) {
		char *data=va_arg(ap, char *);
		len+=strlen(data)+1;
	}
	va_end(ap);
	va_start(ap, carg);
	char *dest=(char*)malloc(len);
	for (i=0; i<carg; i++) {
		char *data=va_arg(ap, char *);
		strcat(dest, data);
	}
	va_end(ap);
//	//printf("%d %s\n", len, dest);
	return dest;
}

/*void recordop(int carg, ...) {
	va_list ap;
	va_start(ap, carg);
	int i;
	for (i=0; i<carg; i++) {
		Operand *
	}
}*/

char *newt() {
	char tmp[11];
	sprintf(tmp, "%d", varno++);
	return mystrcat(2, "t_of_irene_", tmp);
}

char *newlabel() {
	char tmp[11];
	sprintf(tmp, "%d", labelno++);
	return mystrcat(2, "label_of_irene_", tmp);
}

char *lastt() {
	char tmp[11];
	sprintf(tmp, "%d", varno-1);
	return mystrcat(2, "t", tmp);
}

void print_intercodes() {
	FILE * outfile=fopen("out.ir", "w");
	list_head* p1;
	list_foreach(p1, &INTERCODES.iclist) {
		InterCode* target=list_entry(p1, InterCode, iclist);
		fprintf(outfile, "%s\n", target->val);
//		//printf("hey %s\n", target->val);
	}
	//fprintf(outfile, "RETURN #0\n");
}

void Translate_Program(node *nd) {
////printf("s tr_program\n");
	list_init(&INTERCODES.iclist);
	varno=0; labelno=0;
	Translate_ExtDefList(nd->chi[0]);
	print_intercodes();
}

void Translate_ExtDefList(node *nd) {
	if (nd->cn==0) return;
////printf("s tr_extdeflist\n");
	Translate_ExtDef(nd->chi[0]);
	Translate_ExtDefList(nd->chi[1]);
}

void Translate_ExtDef(node * nd) {
	if (strcmp(nd->chi[1]->name, "FunDec")!=0) return;
////printf("s tr extdef\n");
	Translate_FunDec(nd->chi[1]);
	Translate_CompSt(nd->chi[2]);
}

void Translate_FunDec(node *nd) {
////printf("s tr fundec\n");
	char *idval=nd->chi[0]->val;
	InterCode* tmp; newic(tmp);
	tmp->kind=FUNCTIONIC;
	tmp->val = mystrcat(3, "FUNCTION ", idval, " :");
	list_init(&tmp->oplist);
	Operand* top; newop(top);
	mystrcpy(top->val, idval);
	list_add_before(&tmp->oplist, &top->list);
	list_add_before(&INTERCODES.iclist, &tmp->iclist);
	if (nd->cn>3) Translate_VarList(nd->chi[2]);
}

void Translate_VarList(node* nd) {
////printf("s tr varlist\n");
	Translate_ParamDec(nd->chi[0]);
	if (nd->cn>1) Translate_VarList(nd->chi[2]);
}

void Translate_ParamDec(node* nd) {
////printf("s tr paramdec\n");
	Translate_VarDec(nd->chi[1], 3);
}

char* Translate_VarDec(node *nd, int kind) {
////printf("s tr vardec\n");
	if (nd->cn==1) { //ID
		node *idnd=nd->chi[0];
		char *idval=idnd->val;
		if (kind==3) {//param
			InterCode* tmp; newic(tmp);
			tmp->kind=PARAMIC;
			tmp->val=mystrcat(2, "PARAM ", idval);
			recordop(tmp, idval);
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
			return idval;
		}
		else {
			List *sym=find_table(idval);
			int symkind=sym->type->kind;
			if (symkind!=2) return idval;//int d=0;
			list_head* ptr;
			int fieldlen=0;
			list_foreach(ptr, &sym->type->u.structure.field->list) {
				fieldlen+=4;
			}
			char sizest[11];
			sprintf(sizest, "%d", fieldlen);
			InterCode* tmp; newic(tmp);
			tmp->kind=DECIC;
			tmp->val = mystrcat(4, "DEC ", idval, " ", sizest);
			isparam[hash_pjw(idval)] = 1;
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
		}
	}
	else { //array 
		char *idval=Translate_VarDec(nd->chi[0], 1);
		int size=4*atoi(nd->chi[2]->val);
		char sizest[11]; sprintf(sizest, "%d", size);
		InterCode* tmp; newic(tmp);
		tmp->kind=DECIC;
		tmp->val=mystrcat(4, "DEC ", idval, " ", sizest);
		isparam[hash_pjw(idval)] = 1;
		list_add_before(&INTERCODES.iclist, &tmp->iclist);
////printf("in vardec array: %s\n", tmp->val);
		return idval;//return array name
	}	
}

void Translate_CompSt(node *nd) {
////printf("s tr compst\n");
	Translate_DefList(nd->chi[1]);
	Translate_StmtList(nd->chi[2]);
}

void Translate_DefList(node *nd) {
////printf("s tr deflist\n");
	if (nd->cn==0) return;
	Translate_DecList(nd->chi[0]->chi[1]);
	Translate_DefList(nd->chi[1]);
}

void Translate_DecList(node *nd) {
////printf("s tr declist\n");
	Translate_Dec(nd->chi[0]);
	if (nd->cn>1) Translate_DecList(nd->chi[2]);
}

void Translate_Dec(node *nd) {
//printf("s tr dec\n");
	if (nd->cn==1) { // array or struct, if var no intercodes
		Translate_VarDec(nd->chi[0], 4);
		return;
	}
	char *idval=Translate_VarDec(nd->chi[0], 0);//only basic
	InterCode* tmp; newic(tmp);
	tmp->kind=ASSIGNIC;
	char *opexp=Translate_Exp(nd->chi[2]); 
	tmp->val=mystrcat(3, idval, " := ", opexp);
	list_add_before(&INTERCODES.iclist, &tmp->iclist);
}

char *Translate_Exp(node *nd) {
//printf("s tr exp\n");
	char *ans;

	if (nd->cn==3) {
		if (strcmp(nd->chi[1]->name, "ASSIGNOP")==0) {
			char *exp1=Translate_Exp(nd->chi[0]);
			char *exp2=Translate_Exp(nd->chi[2]);
			char *ttmp=newt();
			InterCode* tmp; newic(tmp);
			tmp->kind=ASSIGNIC;
			tmp->val = mystrcat(3, ttmp, " := ", exp2);
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
//printf("%s\n", exp1);
			InterCode* temp; newic(temp);
			temp->kind=ASSIGNIC;
			temp->val = mystrcat(3, exp1, " := ", ttmp);
			list_add_before(&INTERCODES.iclist, &temp->iclist);

			return exp1;
		}

		/*else if (strcmp(nd->chi[1]->name, "AND")==0 || strcmp(nd->chi[1]->name, "OR")==0 ) {
			char *exp1=Translate_Exp(nd->chi[0]);
			char *exp2=Translate_Exp(nd->chi[2]);
			
		}

		else if (strcmp(nd->chi[1]->name, "RELOP")==0) {

		}	*/

		else if (strcmp(nd->chi[1]->name, "PLUS")==0 || strcmp(nd->chi[1]->name, "MINUS")==0 || strcmp(nd->chi[1]->name, "STAR")==0 || strcmp(nd->chi[1]->name, "DIV")==0) {
			char *exp1=Translate_Exp(nd->chi[0]);
			char *exp2=Translate_Exp(nd->chi[2]);
			InterCode* tmp; newic(tmp);
			ans=newt();

			switch (nd->chi[1]->name[0]) {
				case 'P': tmp->kind=PLUSIC; tmp->val=mystrcat(5,  ans, " := ", exp1, " + ", exp2);  break;
				case 'M': tmp->kind=MINUSIC; tmp->val=mystrcat(5, ans, " := ", exp1, " - ", exp2); break;
				case 'S': tmp->kind=STARIC; tmp->val=mystrcat(5, ans, " := ", exp1, " * ", exp2); break;
				case 'D': tmp->kind=DIVIC; tmp->val=mystrcat(5, ans, " := ", exp1, " / ", exp2); break;
				default : printf("strange\n"); return NULL;
			}
//printf("%s is ic\n", tmp->val);
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
////printf("STAR %s\n",ans);
			return ans;
		}

		else if (strcmp(nd->chi[1]->name, "Exp")==0) { //(exp)
////printf("in (exp) %s\n",Translate_Exp(nd->chi[1]));
			return Translate_Exp(nd->chi[1]);
		}

		else if (strcmp(nd->chi[1]->name, "LP")==0) { //call function without param
			node *ndid=nd->chi[0];
			char *idval=ndid->val;
			char *ret=newt();
			if (!strcmp(idval, "read")) {
				InterCode* tmp; newic(tmp);
				tmp->kind=READIC;
				tmp->val = mystrcat(2, "READ ", ret);
				list_add_before(&INTERCODES.iclist, &tmp->iclist);
			}
			else {
				InterCode* tmp; newic(tmp);
				tmp->kind=CALLIC;
				tmp->val = mystrcat(3, ret, " := CALL ", idval);
				list_add_before(&INTERCODES.iclist, &tmp->iclist);
			}
			return ret;
		}
		else { //struct field access
			char *exp=Translate_Exp(nd->chi[0]);
			char *fval=nd->chi[2]->val;
			//printf("******* %s\n", exp);
			Type* exptype=find_table(exp)->type;
			list_head *ptr; int len=0;
			list_foreach(ptr, &exptype->u.structure.field->list) {
				List* entry=list_entry(ptr, List, list);
				if ( !strcmp(entry->name, fval) ){
					break;	
				}
				len+=4;
			}
			char toffset[11]; 
			sprintf(toffset, "%d", len);
			char *taddr=newt();
			InterCode* temp; newic(temp);
			temp->kind=PLUSIC;//不知道是哪一类。。。
			if(!isparam[hash_pjw(exp)])temp->val = mystrcat(5, taddr, " := ", exp, " + #", toffset); 
			else temp->val = mystrcat(5, taddr, " := &", exp, " + #", toffset);
			list_add_before(&INTERCODES.iclist, &temp->iclist);
			return mystrcat(2, "*", taddr);
		}
	}

	else if (nd->cn==4) {
		if (strcmp(nd->chi[0]->name, "ID")==0) { //call function with param
			node *ndid=nd->chi[0];
			char *idval=ndid->val;
			if (!strcmp(idval, "write")) {
				char *arg=Translate_Exp(nd->chi[2]->chi[0]);
				char *wc=newt();
				InterCode* tmp; newic(tmp);
				tmp->kind=ASSIGNIC;
				tmp->val = mystrcat(3, wc, " := ", arg);
				list_add_before(&INTERCODES.iclist, &tmp->iclist);
				InterCode* temp; newic(temp);
				temp->kind=WRITEIC;
				temp->val = mystrcat(2, "WRITE ", wc);
				list_add_before(&INTERCODES.iclist, &temp->iclist);
				return NULL;//void function
			}
			else {
				char *ret=newt();
				InterCode* args; newic(args); list_init(&args->iclist);
				Translate_Args(nd->chi[2], args);
				list_head* ptr;
				list_foreach(ptr, &args->iclist) {
					InterCode *arg=list_entry(ptr, InterCode, iclist);
					InterCode *temp; newic(temp);
					temp->kind=ARGIC;
					if(isparam[hash_pjw(arg->val)])temp->val=mystrcat(2, "ARG &", arg->val);
					else temp->val=mystrcat(2, "ARG ", arg->val);
					list_add_before(&INTERCODES.iclist, &temp->iclist);
				}

				InterCode* tmp; newic(tmp);
				tmp->kind=CALLIC;
				tmp->val = mystrcat(3, ret, " := CALL ", idval);
				list_add_before(&INTERCODES.iclist, &tmp->iclist);

				return ret;
			}
		}
		else { //array access, only one dimension
			char *arrayid=Translate_Exp(nd->chi[0]);
			char *index=Translate_Exp(nd->chi[2]);
			char *toffset=newt();
			InterCode* tmp; newic(tmp);
			tmp->kind=STARIC;//不知道是哪一类。。。
			tmp->val = mystrcat(4, toffset, " := ", index, " * #4");
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
			char *taddr=newt();
			InterCode* temp; newic(temp);
			temp->kind=PLUSIC;//不知道是哪一类。。。
			temp->val = mystrcat(5, taddr, " := &", arrayid, " + ", toffset);
			list_add_before(&INTERCODES.iclist, &temp->iclist);
			return mystrcat(2, "*", taddr);
		}
	} 

	else if (nd->cn == 2) {
		if(strcmp(nd->chi[0]->name, "MINUS")==0) {
			char *ans1 =newt();
			InterCode* tmp; newic(tmp);
			tmp->kind=ASSIGNIC;
			tmp->val = mystrcat(2, ans1, " := #0");
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
		
			ans=newt();
			char *exp=Translate_Exp(nd->chi[1]);
			newic(tmp);
			tmp->kind=MINUSIC;
			tmp->val = mystrcat(5, ans, " := ", ans1, " - ", exp);
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
////printf("%s\n", ans);
			return ans;
		} 
	/*	else {// !exp
			
		}*/
	}

	else if (nd->cn == 1) {
		if(strcmp(nd->chi[0]->name, "ID")==0) {
//printf("%s\n",nd->chi[0]->val);
			return nd->chi[0]->val;		
		} 
		else /*if (strcmp(nd->chi[0]->name, "INT")==0) */{
			ans=newt();
			InterCode* tmp; newic(tmp);
			tmp->kind=ASSIGNIC;
			tmp->val = mystrcat(3, ans, " := #", nd->chi[0]->val);
			list_add_before(&INTERCODES.iclist, &tmp->iclist);
//printf("%s\n",ans);
			return ans;
		}
	}
	return NULL;
}

void Translate_StmtList(node *nd) {
//printf("s tr stmtlist\n");
	if (nd->cn==0) return;
	Translate_Stmt(nd->chi[0]);
	Translate_StmtList(nd->chi[1]);
}

void Translate_Stmt(node *nd) {
//printf("s tr stmt\n");
	if(nd->cn==2) { //exp;
		Translate_Exp(nd->chi[0]);
	}
	else if(nd->cn == 1) {
		Translate_CompSt(nd->chi[0]);
	}
	else if(nd->cn==3) { //return exp;
		char *reval = Translate_Exp(nd->chi[1]);
		InterCode* tmp; newic(tmp);
		tmp->kind=ASSIGNIC;
		char *tmpt=newt();
		tmp->val = mystrcat(3, tmpt, " := ", reval);
		list_add_before(&INTERCODES.iclist, &tmp->iclist);	

		InterCode* temp; newic(temp);
		temp->kind=RETURNIC;
		temp->val = mystrcat(2, "RETURN ", tmpt);
		list_add_before(&INTERCODES.iclist, &temp->iclist);
	}
	else if(nd->cn == 5 && nd->chi[0]->name[0]=='I') {//if () ...
		char *label1=newlabel();
		char *label2=newlabel();
		Translate_Cond(nd->chi[2], label1, label2);
		InterCode* temp; newic(temp);
		temp->kind=LABELIC;
		temp->val = mystrcat(3, "LABEL ", label1, " :");
		list_add_before(&INTERCODES.iclist, &temp->iclist);
		Translate_Stmt(nd->chi[4]);
		InterCode* tmp; newic(tmp);
		tmp->kind=LABELIC;
		tmp->val = mystrcat(3, "LABEL ", label2, " :");
		list_add_before(&INTERCODES.iclist, &tmp->iclist);
	}
	else if (nd->cn==5 && nd->chi[0]->name[0]=='W') {
		char *label1=newlabel();
		char *label2=newlabel();
		char *label3=newlabel();
		InterCode* temp1; newic(temp1);
		temp1->kind=LABELIC;
		temp1->val = mystrcat(3, "LABEL ", label1, " :");
		list_add_before(&INTERCODES.iclist, &temp1->iclist);
		Translate_Cond(nd->chi[2], label2, label3);
		InterCode* temp2; newic(temp2);
		temp2->kind=LABELIC;
		temp2->val = mystrcat(3, "LABEL ", label2, " :");
		list_add_before(&INTERCODES.iclist, &temp2->iclist);
		Translate_Stmt(nd->chi[4]);
		InterCode* tmp; newic(tmp);
		tmp->kind=GOTOIC;
		tmp->val = mystrcat(2, "GOTO ", label1);
		list_add_before(&INTERCODES.iclist, &tmp->iclist);
		InterCode* temp3; newic(temp3);
		temp3->kind=LABELIC;
		temp3->val = mystrcat(3, "LABEL ", label3, " :");
		list_add_before(&INTERCODES.iclist, &temp3->iclist);
	}
	else if(nd->cn == 7) { // if () else ...
		char *label1=newlabel();
		char *label2=newlabel();
		char *label3=newlabel();
		Translate_Cond(nd->chi[2], label1, label2);//code1
		InterCode* temp1; newic(temp1);
		temp1->kind=LABELIC;
		temp1->val = mystrcat(3, "LABEL ", label1, " :");
		list_add_before(&INTERCODES.iclist, &temp1->iclist);//label label1
		Translate_Stmt(nd->chi[4]);//code 2
		InterCode* tmp; newic(tmp);
		tmp->kind=GOTOIC;
		tmp->val = mystrcat(2, "GOTO ", label3);
		list_add_before(&INTERCODES.iclist, &tmp->iclist);//goto label3
		InterCode* temp2; newic(temp2);
		temp2->kind=LABELIC;
		temp2->val = mystrcat(3, "LABEL ", label2, " :");
		list_add_before(&INTERCODES.iclist, &temp2->iclist);//label label2
		Translate_Stmt(nd->chi[6]);//code3
		InterCode* temp3; newic(temp3);
		temp3->kind=LABELIC;
		temp3->val = mystrcat(3, "LABEL ", label3, " :");
		list_add_before(&INTERCODES.iclist, &temp3->iclist);//label label3
	}
}

void Translate_Cond(node *nd, char* label_true, char* label_false) {
	if (nd->cn==2 && !strcmp(nd->chi[0]->name, "NOT")) {
		Translate_Cond(nd->chi[1], label_false, label_true);
	}
	else if (nd->cn==3 && nd->chi[1]->name[0]=='R') {
		char *relop=nd->chi[1]->val;
//printf("relop: %s\n", relop); 
		char *exp1=Translate_Exp(nd->chi[0]);
		char *exp2=Translate_Exp(nd->chi[2]);
		InterCode* tmp; newic(tmp);
		tmp->kind=IFIC;
		tmp->val = mystrcat(8, "IF ", exp1, " ", relop, " ", exp2, " GOTO ",label_true);
		list_add_before(&INTERCODES.iclist, &tmp->iclist);
		InterCode* tmpf; newic(tmpf);
		tmpf->kind=GOTOIC;
		tmpf->val=mystrcat(2, "GOTO ", label_false);
		list_add_before(&INTERCODES.iclist, &tmpf->iclist);
	}
	else if (nd->cn==3 && nd->chi[1]->name[0]=='A') {
		char *label1=newlabel();
		Translate_Cond(nd->chi[0], label1, label_false);
		InterCode* temp1; newic(temp1);
		temp1->kind=LABELIC;
		temp1->val = mystrcat(3, "LABEL ", label1, " :");
		list_add_before(&INTERCODES.iclist, &temp1->iclist);
		Translate_Cond(nd->chi[2], label_true, label_false);
	}
	else if (nd->cn==3 && nd->chi[1]->name[0]=='O') {
		char *label1=newlabel();
		Translate_Cond(nd->chi[0], label_true, label1);
		InterCode* temp1; newic(temp1);
		temp1->kind=LABELIC;
		temp1->val = mystrcat(3, "LABEL ", label1, " :");
		list_add_before(&INTERCODES.iclist, &temp1->iclist);
		Translate_Cond(nd->chi[2], label_true, label_false);
	}
	else {
		char *exp1=Translate_Exp(nd);
		InterCode* temp1; newic(temp1);
		temp1->kind=IFIC;
		temp1->val = mystrcat(4, "IF ", exp1, " != #0 GOTO ", label_true);
		list_add_before(&INTERCODES.iclist, &temp1->iclist);
		InterCode* temp; newic(temp);
		temp->kind=GOTOIC;
		temp->val = mystrcat(2, "GOTO ", label_false);
		list_add_before(&INTERCODES.iclist, &temp->iclist);
	}
}

void Translate_Args(node *nd, InterCode *args) {
	if (nd->cn>1) {
		Translate_Args(nd->chi[2], args);
	}
	char *exp=Translate_Exp(nd->chi[0]);
	InterCode *data; newic(data);
	mystrcpy(data->val, exp);
	list_add_before(&args->iclist, &data->iclist);
}
