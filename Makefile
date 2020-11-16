.PHONY: debug-common debug-arraylist O-common debug-main

CFLAGS=-std=gnu99 -Wextra -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-duplicate-decl-specifier 
CFLAGS_STRICT=-std=c99 -Wextra -Wall -Werror -pedantic
LIBS=-lm -lpthread

debug-common:
	gcc -g $(CFLAGS) $(LIBS) $(file) ./common/*.c -o ./common/test-run

test-arraylist: file=./test/arraylist_test.c
test-arraylist: debug-common
	./common/test-run

test-queue: file=./test/queue_test.c
test-queue: debug-common
	./common/test-run

test-vector3: file=./test/vector3_test.c
test-vector3: debug-common
	./common/test-run

genannCSV: 
	gcc $(CFLAGS_STRICT) ./genann/genannCSV.c ./genann/genann.c ./utils/timer/timer.c ./common/arraylist.c -lm -o ML
	./ML
	rm ML

accuracyTesting: 
	gcc $(CFLAGS_STRICT) ./genann/pursuitOfAccuracy.c ./genann/genann.c ./common/arraylist.c -lm -o ML
	./ML
	rm ML

test-array2d: file=./test/array2d_test.c
test-array2d: debug-common
	./common/test-run

O-common:
	gcc -O3 $(CFLAGS) $(LIBS) $(file) ./common/*.c -o test-O

bin/sensehat.o: 
	gcc $(CFLAGS) $(LIBS) -lrt -c ./lib/sensehat/sensehat.c -o ./bin/sensehat.o

bin/mqtt.o:
	gcc $(CFLAGS) $(LIBS) -lrt -I./lib/MQTT-C/include -c ./lib/MQTT-C/src/mqtt.c -o ./bin/mqtt.o

bin/mqtt_pal.o:
	gcc $(CFLAGS) $(LIBS) -lrt -I./lib/MQTT-C/include -c ./lib/MQTT-C/src/mqtt_pal.c -o ./bin/mqtt_pal.o

bin/genann.o:
	gcc $(CFLAGS) $(LIBS) -lrt -c ./genann/genann.c -o ./bin/genann.o

main.h.gch:
	gcc main.h -I./lib/MQTT-C/include

debug-main: bin/sensehat.o bin/mqtt.o bin/mqtt_pal.o bin/genann.o main.h.gch
	gcc -g $(CFLAGS) $(LIBS) -lrt -lwiringPi \
	./bin/sensehat.o \
	-I./lib/MQTT-C/include ./bin/mqtt.o ./bin/mqtt_pal.o \
	./bin/genann.o ./genann/classifier.c \
	./common/*.c ./utils/*.c \
	./main.c -o ./bin/main

test-main: debug-main
	./bin/main

clean:
	rm main.h.gch
	rm -rf ./bin/*.o
