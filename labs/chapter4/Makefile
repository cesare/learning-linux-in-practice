CC=clang-4.0

CFLAGS=-g -std=c11 -Werror -Weverything -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=199309L

EXECUTABLES = \
	sched \
	sched_nice

all: $(EXECUTABLES)

sched: sched.c
	$(CC) $(CFLAGS) -o $@ $^

sched_nice: sched_nice.c
	$(CC) $(CFLAGS) -o $@ $^

tsv: sched sched_nice
	mkdir -p results
	taskset -c 0 ./sched 1 100 1 > results/1core-1process.tsv
	taskset -c 0 ./sched 2 100 1 > results/1core-2process.tsv
	taskset -c 0 ./sched 4 100 1 > results/1core-4process.tsv
	taskset -c 0,2 ./sched 1 100 1 > results/ex4-d.tsv
	taskset -c 0,2 ./sched 2 100 1 > results/ex4-e.tsv
	taskset -c 0,2 ./sched 4 100 1 > results/ex4-f.tsv
	taskset -c 0 ./sched_nice 100 1 > results/sched_nice.tsv
clean:
	-rm -f $(EXECUTABLES)
	-rm -rf results/
