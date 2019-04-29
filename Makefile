all:
	gcc -o programaTrab1 main.c -g
run:
	./programaTrab1
debug:
	gdb programaTrab1
val:
	valgrind --leak-check=full --track-origins=yes ./programaTrab1
zip:
	zip trab1.zip main.c Makefile
