%{
	int yycolumn = 1;
	#define YY_USER_ACTION \
	yylloc.first_line = yylloc.last_line = yylineno; \
	yylloc.first_column = yycolumn; \
	yylloc.last_column = yycolumn + yyleng - 1; \
	yycolumn += yyleng;
%}

%{
	#include "tree.h"
	#include <stdio.h>
	#include "syntax.tab.h"
//	#define YYSTYPE node
	extern int error_flag;
%}
%option yylineno
letter [a-zA-Z]
digit [0-9]
part_digit 1|2|3|4|5|6|7|8|9
INT {digit}|{part_digit}{digit}+
FLOAT ({digit}|{part_digit}{digit}+)\.{digit}+
ID ({letter}|_)+({letter}|_|{digit})*
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
NOT \!
TYPE "int"|"float"
STRUCT "struct"
RETURN "return"
IF "if"
ELSE "else"
WHILE "while"
BLANK [\f|\r|\t|\v|" "]+


%%
{TYPE} { node *tmp = create_node("TYPE", yytext, 0); 	tmp->hvl = 1;
	/*if (strcmp(yytext, "int")) tmp->val = "float"; 
	else tmp->val = "int";*/
	yylval.NODE = tmp;
	return TYPE; printf("TYPE\n"); }
{STRUCT} {node *tmp = create_node("STRUCT","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return STRUCT; printf("struct\n"); }
{RETURN} {node *tmp = create_node("RETURN","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return RETURN; printf("return\n"); }
{IF} { node *tmp = create_node("IF", "-1",0); tmp->hvl = 0; yylval.NODE=tmp; 
	return IF; printf("if\n"); }
{ELSE} {node *tmp = create_node("ELSE","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return ELSE; printf("else\n"); }
{WHILE} { node *tmp = create_node("WHILE", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return WHILE; printf("while\n"); }
{RELOP} { node *tmp = create_node("RELOP", yytext, 0); tmp->hvl= 0; yylval.NODE=tmp;
	return RELOP; printf("relop\n"); }
{FLOAT} {	node *tmp = create_node("FLOAT", yytext, 0); tmp->hvl = 1; yylval.NODE= tmp;
//	yylval = atof(yytext);
	return FLOAT;
	printf("float\n"); }
{INT} {	//yylval = atoi(yytext); 
	node *tmp = create_node("INT", yytext, 0); tmp->hvl = 1; yylval.NODE = tmp;
	return INT;
	printf("int\n"); 
}
{ID} {node *tmp = create_node("ID", yytext, 0); tmp->hvl = 1; yylval.NODE = tmp;
	return ID; printf("id\n"); } 
&& {node *tmp = create_node("AND","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return AND; printf("&&\n"); }
\|\| {printf("||\n");node *tmp = create_node("OR","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return OR; printf("||\n"); }
\. {node *tmp = create_node("DOT", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return DOT; printf("dot\n"); }
{NOT} {node *tmp = create_node("NOT", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return NOT; printf("Not\n"); }
\* {node *tmp = create_node("STAR","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return STAR; printf("*\n"); }
\/ {node *tmp = create_node("DIV", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return DIV; printf("/\n"); }
; { node *tmp = create_node("SEMI", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return SEMI; printf(";\n"); }
, {node *tmp = create_node("COMMA","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return COMMA; printf(",\n"); }
= {node *tmp = create_node("ASSIGNOP", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return ASSIGNOP; printf("assign\n"); }
\+ {node *tmp = create_node("PLUS","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return PLUS; printf("+\n"); }
- {node *tmp = create_node("MINUS","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return MINUS; printf("-\n"); }
\( { node *tmp = create_node("LP", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return LP; printf("(\n"); }
\) {node *tmp = create_node("RP","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return RP; printf(")\n"); }
\[ {node *tmp = create_node("LB", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return LB; printf("[\n"); }
\] {node *tmp = create_node("RB", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return RB; printf("]\n"); }
\{ {node *tmp = create_node("LC", "-1",0); tmp->hvl = 0; yylval.NODE=tmp;
	return LC; printf("{\n"); }
\} { node *tmp = create_node("RC","-1", 0); tmp->hvl = 0; yylval.NODE=tmp;
	return RC; printf("}\n"); }
\n { yycolumn = 1; 
	//printf("\\n\n"); 
	}
" "|\t {}
. {
	error_flag=1;
	printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
	}
%%
/*int main(int argc, char** argv) {
	if (argc > 1) {
		if (!(yyin = fopen(argv[1], "r"))) {
			perror(argv[1]);
			return 1;
		}
	}
	yylineno=1;
	yylex();
	return 0;
}
*/
