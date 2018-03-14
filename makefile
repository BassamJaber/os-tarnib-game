child main : child.c main.c header.c header.h
	gcc main.c -o main
	gcc child.c -o child

