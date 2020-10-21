CFLAGS=-std=c99 -lm -Wextra -Werror -pedantic
CFLAGS_STRICT=-std=c99 -lm -Wextra -Werror -pedantic
LIBS=-lpthread -lm

test-common:
	gcc $(CFLAGS) $(LIBS) $(file) ./common/*.c -o test
