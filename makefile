editor:main.o editor.o
	gcc main.o editor.o text_operations.o -o editor

main.o:main.c
	gcc main.c -std=c99 -c -Wall -g -o main.o
editor.o:editor.c
	gcc -c editor.c -std=c99 -Wall -g -o editor.o
	
text_operations.o:text_operations.c
	gcc -c text_operations.c -std=c99 -Wall -g -o text_operations.o