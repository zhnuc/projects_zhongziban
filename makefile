editor:main.o editor.o
	gcc main.o editor.o -o editor

main.o:main.c
	gcc main.c -std=c99 -c -Wall -g -o main.o
editor.o:editor.c
	gcc -c editor.c -std=c99 -Wall -g -o editor.o