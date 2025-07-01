editor:main.o
	gcc main.o -o editor

main.o:main.c
	gcc main.c -c -Wall -g -o main.o
	