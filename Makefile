.PHONY: debug-common debug-arraylist O-common debug-main

# C compiler flags with extra warnings, using GNU C99 as MQTT requires posix sockets which needs GNU C support 
CFLAGS=-std=gnu99 -Wextra -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-duplicate-decl-specifier 
CFLAGS_STRICT=-std=c99 -Wextra -Wall -Werror -pedantic

# Standard runtime library dependencies 
LIBS=-lm -lpthread -lrt -lwiringPi

# Precompiled 3rd party libraries to object files and also the main header
PREBUILDS=bin/mqtt.o bin/mqtt_pal.o \
	bin/genann.o \
	main.h.gch

# The references to the precompiled object files from 3rd party libaries 
ARTIFACTS=-I ./lib/MQTT-C/include ./bin/mqtt.o ./bin/mqtt_pal.o \
	./bin/genann.o
	
# The actual project source files to be always recompiled everytime
TARGETS=./lib/sensehat/sensehat.c \
	./genann/combined_classifier.c \
	./common/*.c \
	./utils/*.c \
	./main.c 

# Full build with optimisation
default: $(PREBUILDS)
	gcc -O3 $(CFLAGS) $(LIBS) \
	$(ARTIFACTS) \
	$(TARGETS) \
	-o ./bin/fallsafe

# Full build with GDB debugging and with "#define DEBUG" directive to enable more print outs 
debug-main: $(PREBUILDS)
	gcc -g $(CFLAGS) $(LIBS) \
	$(ARTIFACTS) \
	$(TARGETS) \
	-D DEBUG -o ./bin/fallsafe_debug

# Shorthand to debug build and run
test-main: debug-main
	./bin/fallsafe_debug

clean-build: clean
	gcc -O3 \
	-lm -lpthread -lrt -lwiringPi \
	./genann/genann.c ./genann/combined_classifier.c \
	-I ./lib/MQTT-C/include ./lib/MQTT-C/src/mqtt.c ./lib/MQTT-C/src/mqtt_pal.c \
	./lib/sensehat/sensehat.c \
	./common/*.c \
	./utils/*.c \
	./main.c -o bin/fallsafe

# Clean all the build artifacts and binaries
clean:
	find "bin/" -type f -not -name '.keep' -print0 | xargs -0 rm -f --
	rm -f main.h.gch

# Third party library prebuild object files
bin/mqtt.o:
	gcc $(CFLAGS) $(LIBS) -I ./lib/MQTT-C/include -c ./lib/MQTT-C/src/mqtt.c -o ./bin/mqtt.o

bin/mqtt_pal.o:
	gcc $(CFLAGS) $(LIBS) -I ./lib/MQTT-C/include -c ./lib/MQTT-C/src/mqtt_pal.c -o ./bin/mqtt_pal.o

bin/genann.o:
	gcc $(CFLAGS) $(LIBS) -c ./genann/genann.c -o ./bin/genann.o

# Precompiled header
main.h.gch:
	gcc main.h -I./lib/MQTT-C/include

# Debug build our own written common data structures and helpers files 
debug-common:
	gcc -g $(CFLAGS) $(LIBS) $(file) -D DEBUG ./common/*.c -o ./common/test-run

# Build with optimisation with own written common data structures and helpers files 
O-common:
	gcc -O3 $(CFLAGS) $(LIBS) $(file) ./common/*.c -o test-O

# Debug build and test runs our own written common data structures and helpers files 
test-arraylist: file=./test/arraylist_test.c
test-arraylist: debug-common
	./common/test-run

test-queue: file=./test/queue_test.c
test-queue: debug-common
	./common/test-run

test-vector3: file=./test/vector3_test.c
test-vector3: debug-common
	./common/test-run

test-array2d: file=./test/array2d_test.c
test-array2d: debug-common
	./common/test-run

# Machine learning training model generation util
genannCSV: 
	gcc $(CFLAGS_STRICT) ./genann/genannCSV.c ./genann/genann.c ./utils/timer/timer.c ./common/arraylist.c -lm -o ML
	./ML
	rm ML

# Machine learning accuracy testing util
accuracyTesting: 
	gcc $(CFLAGS_STRICT) ./genann/pursuitOfAccuracy.c ./genann/genann.c ./common/arraylist.c -lm -o ML
	./ML
	rm ML


