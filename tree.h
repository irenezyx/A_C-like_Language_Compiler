#ifndef _TREE_H
#define _TREE_H

typedef struct syntax_tree_node {
	char *name; //"program"...
	int hvl; //=1:has value
	char *val;
	int line;
	struct syntax_tree_node *chi[8];
	struct syntax_tree_node *par;
	int cn; //current number of children
} node;

node *create_node(char *, char *, int );
void create_edge(node *, node *);
void print_tree(node *, int);

#endif
