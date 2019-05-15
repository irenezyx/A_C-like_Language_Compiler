#include <stdio.h>
#include "syntax.tab.h"
#include "tree.h"
#include "table.h"
#include "intercode.h"
#include "mip32.h"

int yyparse(void);
int yyrestart(void*);
extern int error_flag;
extern node* root;
extern FILE* outs;
int main(int argc, char** argv) {
/*	if (argc<=1) return 1;
	FILE* f=fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]);
		return 1;
	} 
	yyrestart(f);
*/	error_flag = 0;
	freopen(argv[1], "r", stdin);
	if(argc > 2) outs = fopen(argv[2], "w");
	else outs = fopen("out.s", "w");
	yyparse();
	if (!error_flag) {
	//	print_tree(root, 0);
		init_table();
		Program(root);
		Translate_Program(root);
		Translate_asm();
	}
	return 0;
}
