CC=clang-4.0

CFLAGS=-g -std=c11 -Werror -Weverything -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=199309L

EXECUTABLES = \
	segv

all: $(EXECUTABLES)

segv: segv.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm -f $(EXECUTABLES)
	-rm -rf results/
