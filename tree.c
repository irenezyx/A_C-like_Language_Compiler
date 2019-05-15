#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

node *create_node(char *exp, char *valu, int line) {
	node *n = malloc(sizeof(node));
	n->line = line;
	char *tmp = malloc(sizeof(exp));
	strcpy(tmp, exp);
	n->name = tmp;
	char *tp = malloc(sizeof(valu));
	strcpy(tp, valu);
	n->val = tp;
	int i;
	for (i = 0; i < 8; i++) n->chi[i] = NULL;
	n->cn = 0;
	n->par = n;
	return n;
}

void create_edge(node *parent, node* child) {
	if (parent->cn > 8) {
		printf("Too many children!\n");
		return;
	}
	parent->chi[parent->cn] = child;
	parent->cn++;
	if (child!=NULL) child->par = parent;//child might be null str
}

void print_tree(node *nd, int cot) {
	if (nd == NULL) return;
	int i;
	for (i=0; i<cot; i++) { printf("  "); }

	if (!nd->cn) { //token
		printf("%s", nd->name); //not with value
		if (nd->hvl) {
			printf(": ");
			if (strcmp(nd->name, "TYPE") == 0) printf("%s\n", nd->val);
			else if (strcmp(nd->name, "INT") == 0) printf("%d\n", atoi(nd->val));
			else if (strcmp(nd->name, "FLOAT")==0) printf("%f\n", atof(nd->val));
			else printf("%s\n", nd->val);
		}
		else printf("\n");
	}

	else { //not zhongjie
		printf("%s (%d)\n", nd->name, nd->line);
		int j;
		for (j = 0; j < (nd->cn); j ++) {
			print_tree(nd->chi[j], cot+1);
		}
	}
}
