all:
	gcc -c common.c -lpthread
	gcc -pthread equipment.c common.o -o equipment -lpthread
	gcc -pthread server.c common.o -o server -lm -lpthread
