all:
	flex lexical.l
	bison -d syntax.y
	gcc -g main.c tree.c syntax.tab.c table.c intercode.c mip32.c -lfl -ly -o parser

f = test.cmm
o = out.s
test: all
	./parser $(f) $(o)
	spim -file $(o)
	
submit: 
	rm -f lex.yy.c syntax.tab* *.s *.ir
	cd .. && zip -r 141220149_lab4.zip $(shell pwd | grep -o '[^/]*$$')
