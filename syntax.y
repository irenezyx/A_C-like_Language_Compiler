%locations

%{
	#include "lex.yy.c"
	#include <stdio.h>
	#include "tree.h"

	node *root = NULL;
	int error_flag = 0;
%}

%union {
	struct syntax_tree_node *NODE;
};

%token <NODE> INT
%token <NODE> FLOAT
%token <NODE> PLUS MINUS STAR DIV
%token <NODE> ID SEMI COMMA ASSIGNOP RELOP AND OR DOT NOT TYPE LP RP LB RB LC RC
%token <NODE> STRUCT RETURN IF ELSE WHILE

%right ASSIGNOP  
%left OR AND 
%left RELOP 
%left PLUS MINUS 
%left STAR DIV
%right NOT
%left DOT LB RB LP RP
%nonassoc LOWER_THAN_ELSE 
%nonassoc ELSE
%nonassoc STRUCT RETURN WHILE

%type <NODE> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args

%%
Program : ExtDefList {
		root = $$ = create_node("Program","-1", @$.first_line);
		$$->hvl = 0; create_edge($$, $1);
	}
	;
ExtDefList : /* empty */ { $$= create_node("ExtDefList", "-1", @$.first_line); $$->hvl=0; 
	}
	| ExtDef ExtDefList { 
		$$ = create_node("ExtDefList", "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2);
	}
	;
ExtDef : Specifier ExtDecList SEMI {
			 $$ = create_node("ExtDef", "-1", @$.first_line); $$->hvl=0;
			 create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
		 }	 
	| Specifier SEMI {
		$$ = create_node("ExtDef", "-1", @$.first_line); $$->hvl = 0;
		create_edge($$, $1); create_edge($$, $2);
	}
	| Specifier FunDec CompSt {
		$$ = create_node("ExtDef", "-1", @$.first_line); $$->hvl = 0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
	}
	| Specifier FunDec SEMI {
		$$ = create_node("ExtDef", "-1", @$.first_line); $$->hvl = 0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
	}	
	
	;
ExtDecList : VarDec { 
				 $$ = create_node("ExtDecList", "-1", @$.first_line);
				 $$->hvl = 0; create_edge($$, $1);
			 }
	| VarDec COMMA ExtDecList {
		$$ = create_node("ExtDecList", "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
	}
	;

Specifier : TYPE {
				$$=create_node("Specifier", "-1", @$.first_line);
				$$->hvl=0; create_edge($$, $1);
			}
	| StructSpecifier {
		$$=create_node("Specifier", "-1", @$.first_line);
				$$->hvl=0; create_edge($$, $1);
	}
	;
StructSpecifier : STRUCT OptTag LC DefList RC {
					  $$ = create_node("StructSpecifier", "-1", @$.first_line); $$->hvl=0; 
					  create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
					  create_edge($$, $4); create_edge($$, $5);
				  }
	| STRUCT Tag {
		$$ = create_node("StructSpecifier", "-1", @$.first_line); $$->hvl=0; 
		create_edge($$, $1); create_edge($$, $2); 
	}
	;
OptTag : ID {
			$$= create_node("OptTag", "-1", @$.first_line); $$->hvl = 0;
			 create_edge($$, $1);
		 }
	| { $$ = create_node("OptTag", "-1", @$.first_line); $$->hvl = 0;
 }
	;
Tag : ID {
		  $$= create_node("Tag", "-1", @$.first_line); $$->hvl = 0;
		  create_edge($$, $1);
	  }
	;

VarDec : ID  {
			$$=  create_node("VarDec" , "-1", @$.first_line); $$->hvl=0;
			 create_edge($$, $1);
		 }
	| VarDec LB INT RB {
		$$= create_node("VarDec" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3); create_edge($$, $4);
		}
	;
FunDec : ID LP VarList RP {
		$$= create_node("FunDec" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3); create_edge($$, $4);
		}
	| ID LP RP {
		$$= create_node("FunDec" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3); 
	}
	;
VarList : ParamDec COMMA VarList {
		$$= create_node("VarList" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3); 
		  }
	| ParamDec {
			$$= create_node("VarList" , "-1", @$.first_line); $$->hvl=0;
			create_edge($$, $1); 
	}
	;
ParamDec : Specifier VarDec {
			$$= create_node("ParamDec" , "-1", @$.first_line); $$->hvl=0;
			create_edge($$, $1); create_edge($$, $2);
		   }
	;

CompSt : LC DefList StmtList RC {
		$$= create_node("CompSt" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3); 
		create_edge($$, $4);
		 }
	;
StmtList : Stmt StmtList {
		$$= create_node("StmtList" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2);
		   }
	| {$$=create_node("StmtList" , "-1", @$.first_line); $$->hvl=0;
} 
	;
Stmt : Exp SEMI {
		$$= create_node("Stmt" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2);
	   }
	| CompSt {
		$$= create_node("Stmt" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); 
	   }

	| RETURN Exp SEMI{
		$$= create_node("Stmt" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
	   }

	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
		$$= create_node("Stmt" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
		create_edge($$, $4); create_edge($$, $5); 
	   }

	| IF LP Exp RP Stmt ELSE Stmt{
		$$= create_node("Stmt" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2);create_edge($$, $3);
		create_edge($$, $4); create_edge($$, $5); create_edge($$, $6);
		create_edge($$, $7);
	   }

	| WHILE LP Exp RP Stmt{
		$$= create_node("Stmt" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
		create_edge($$, $4); create_edge($$, $5); 
	   }
	| error SEMI
	;

DefList : Def DefList{
		$$= create_node("DefList" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); 
	   }

	| {$$=create_node("DefList" , "-1", @$.first_line); $$->hvl=0;
} 
	;
Def : Specifier DecList SEMI{
		$$= create_node("Def" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
	   }
	| error SEMI

	;
DecList : Dec{
		$$= create_node("DecList" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); 
	   }

	| Dec COMMA DecList{
		$$= create_node("DecList" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
	   }

	;
Dec : VarDec{
		$$= create_node("Dec" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); 
	   }

	| VarDec ASSIGNOP Exp{
		$$= create_node("Dec" , "-1", @$.first_line); $$->hvl=0;
		create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);
	   }

	;

Exp : Exp ASSIGNOP Exp {$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp AND Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp OR Exp{printf("***\n");$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp RELOP Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp PLUS Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp MINUS Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp STAR Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp DIV Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| LP Exp RP{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| MINUS Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); }
	| NOT Exp{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); }
	| ID LP Args RP{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3); create_edge($$, $4);}
	| ID LP RP{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp LB Exp RB{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);create_edge($$, $4);}
	| Exp DOT ID{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| ID{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1);}
	| INT{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); }
	| FLOAT{$$= create_node("Exp", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); }
	;
Args : Exp COMMA Args{$$= create_node("Args", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); create_edge($$, $2); create_edge($$, $3);}
	| Exp {$$= create_node("Args", "-1", @$.first_line); $$->hvl=0; create_edge($$, $1); }
	;

%%

/*#include "lex.yy.c"

int main() {
	yyparse();
}
*/

int yyerror(char* msg) {
	error_flag ++;
	fprintf(stderr, "Error type B at Line %d: %s about \"%s\"\n", yylineno, msg, yytext);

//	printf("Error type B at Line %d: %s around \"%s\"",  
	return 0;
}

