.PHONY: debug-common debug-arraylist O-common debug-main

CFLAGS=-std=c99 -lm -Wextra -pedantic
CFLAGS_STRICT=-std=c99 -lm -Wextra -Werror -pedantic
LIBS=-lpthread -lm

debug-common:
	gcc -g $(CFLAGS) $(LIBS) $(file) ./common/*.c -o test

debug-arraylist: file=./test/arraylist_test.c 
debug-arraylist: debug-common
	./test.exe

O-common:
	gcc -O3 $(CFLAGS) $(LIBS) $(file) ./common/*.c -o test-O

debug-main:
	gcc -g $(CFLAGS) $(LIBS) ./common/*.c ./utils/*.c main.c -o main
