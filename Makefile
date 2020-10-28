.PHONY: debug-common debug-arraylist O-common debug-main

CFLAGS=-std=c99 -lm -Wextra -pedantic
CFLAGS_STRICT=-std=c99 -lm -Wextra -Werror -pedantic
LIBS=-lpthread -lm

debug-common:
	gcc -g $(CFLAGS) $(LIBS) $(file) ./common/*.c -o ./test-run

test-arraylist: file=./test/arraylist_test.c
test-arraylist: debug-common
	./test-run

test-queue: file=./test/queue_test.c
test-queue: debug-common
	./test-run

test-vector3: file=./test/vector3_test.c
test-vector3: debug-common
	./test-run

genannCSV: 
	gcc $(CFLAGS_STRICT) ./genann/genannCSV.c ./genann/genann.c ./common/arraylist.c -lm -o ML
	./ML
	rm ML

accuracyTesting: 
	gcc $(CFLAGS_STRICT) ./genann/pursuitOfAccuracy.c ./genann/genann.c ./common/arraylist.c -lm -o ML
	./ML
	rm ML

O-common:
	gcc -O3 $(CFLAGS) $(LIBS) $(file) ./common/*.c -o test-O

debug-main:
	gcc -g $(CFLAGS) $(LIBS) ./common/*.c ./utils/*.c main.c -o main
