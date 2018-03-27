CC=clang-4.0

CFLAGS=-g -std=c11 -Werror -Weverything -D_POSIX_C_SOURCE=199309L

EXECUTABLES = \
	sched

all: $(EXECUTABLES)

sched: sched.c
	$(CC) $(CFLAGS) -o $@ $^

tsv: sched
	-mkdir -p results
	-taskset -c 0 ./sched 1 100 1 > results/1core-1process.tsv
	-taskset -c 0 ./sched 2 100 1 > results/1core-2process.tsv
	-taskset -c 0 ./sched 4 100 1 > results/1core-4process.tsv

clean:
	-rm -f $(EXECUTABLES)
	-rm -rf results/
