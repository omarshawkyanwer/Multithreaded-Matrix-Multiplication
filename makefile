Exe:	matmul.o
	gcc -pthread -o Exe matmul.o

matmul.o:	matmul.c
	gcc -c matmul.c
