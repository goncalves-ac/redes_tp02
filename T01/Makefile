all:
	gcc -Wall -c common.c
	gcc -Wall -pthread equipamento.c common.o -o equipamento
	gcc -Wall -pthread server.c common.o -o server -lm
