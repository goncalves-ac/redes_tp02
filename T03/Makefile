all:
	gcc -Wall -c common.c
	gcc -Wall -pthread eq.c common.o -o eq
	gcc -Wall -pthread s-mt.c common.o -o s-mt -lm
