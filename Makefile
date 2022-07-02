all:
	gcc -c common.c
	gcc -pthread equipment.c common.o -o equipment
	gcc -pthread server.c common.o -o server -lm
