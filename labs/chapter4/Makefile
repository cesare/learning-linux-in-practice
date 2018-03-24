CC=clang-4.0

CFLAGS=-g -std=c11 -Werror -Weverything -D_POSIX_C_SOURCE=199309L

EXECUTABLES = \
	sched

all: $(EXECUTABLES)

sched: sched.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm -f $(EXECUTABLES)
