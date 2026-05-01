# Compiler
CC = gcc

# Standard Options 
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -Wconversion 

# Style Options
CFLAGS += -Wshadow -Wstrict-prototypes -Wmissing-declarations 

# Safety & Security Options
CFLAGS += -fstack-protector-strong -fwrapv

ifeq ($(OPTIMIZE), 1)
	CFLAGS += -O3 -DNDEBUG
else
	CFLAGS += -g -O0 -DDEBUG
endif 

CFLAGS += -I include -I test

all: bin/test_cset bin/test_buf

test: all test_cset test_buf
	 
bin:
	mkdir bin

build: 
	mkdir build 

clean:
	rm -rf build || true   
	rm -rf bin || true 

bin/test_cset: test/test_cset.c include/pgenc/cset.h bin
	$(CC) $(CFLAGS) -o $@ $<
test_cset: bin/test_cset
	valgrind -q --error-exitcode=1 --leak-check=full $^ 

bin/test_buf: test/test_buf.c include/pgenc/buf.h bin
	$(CC) $(CFLAGS) -o $@ $<
test_buf: bin/test_buf
	valgrind -q --error-exitcode=1 --leak-check=full $^ 

optimize: 
	$(MAKE) OPTIMIZE=1 all
