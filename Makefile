all:
	gcc -Wall -c common.c
	gcc -Wall -pthread equipment.c common.o -o equipment
	gcc -Wall  server.c common.o -o server -lm
