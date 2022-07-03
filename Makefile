all:
	gcc -Wall -c common.c -lpthread
	gcc -Wall equipment.c common.o -o equipment -lpthread
	gcc -Wall server.c common.o -o server -lpthread
