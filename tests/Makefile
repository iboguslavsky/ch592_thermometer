CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2 -I../APP/include

all: test_bthome_temp
	./test_bthome_temp

test_bthome_temp: test_bthome_temp.c ../APP/bthome_temp.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f test_bthome_temp

.PHONY: all clean
