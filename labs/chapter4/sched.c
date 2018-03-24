#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NLOOP_FOR_ESTIMATION 1000000000UL  // 1,000,000,000
#define NSECS_PER_MSEC          1000000UL  //     1,000,000
#define NSECS_PER_SEC        1000000000UL  // 1,000,000,000

/*
 * before should be <= after
 */
static inline uint64_t diff_nsec(struct timespec before, struct timespec after) {
  return (((uint64_t) after.tv_sec * NSECS_PER_SEC + (uint64_t) after.tv_nsec)
        - ((uint64_t)before.tv_sec * NSECS_PER_SEC + (uint64_t)before.tv_nsec));
}

static uint64_t loops_per_msec() {
  struct timespec before;
  clock_gettime(CLOCK_MONOTONIC, &before);

  for (uint64_t i = 0; i < NLOOP_FOR_ESTIMATION; i++);

  struct timespec after;
  clock_gettime(CLOCK_MONOTONIC, &after);

  return NLOOP_FOR_ESTIMATION * NSECS_PER_MSEC / diff_nsec(before, after);
}

static inline void load(uint64_t nloop) {
  for (uint64_t i = 0; i < nloop; i++);
}

static noreturn void child_fn(uint64_t id, struct timespec* buf, uint64_t nrecord, uint64_t nloop_per_resol, struct timespec start) {
  for (uint64_t i = 0; i < nrecord; i++) {
    struct timespec ts;
    load(nloop_per_resol);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    buf[i] = ts;
  }

  for (uint64_t i = 0; i < nrecord; i++) {
    fprintf(stdout, "%ld\t%ld\t%ld\n", id, diff_nsec(start, buf[i]) / NSECS_PER_MSEC, (i + 1) * 100 / nrecord);
  }

  exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <nproc> <total[ms]> <resolution[ms]>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int64_t nproc = strtol(argv[1], NULL, 10);
  int64_t total = strtol(argv[2], NULL, 10);
  int64_t resol = strtol(argv[3], NULL, 10);

  if (nproc < 1) {
    fprintf(stderr, "<nproc>(%ld) should be >= 1\n", nproc);
    exit(EXIT_FAILURE);
  }

  if (total < 1) {
    fprintf(stderr, "<total>(%ld) should be >= 1\n", total);
    exit(EXIT_FAILURE);
  }

  if (resol < 1) {
    fprintf(stderr, "<resol>(%ld) should be >= 1\n", resol);
    exit(EXIT_FAILURE);
  }

  if (total % resol != 0) {
    fprintf(stderr, "<total>(%ld) should be multiple of <resolution>(%ld)\n", total, resol);
    exit(EXIT_FAILURE);
  }

  int64_t nrecord = total / resol;

  puts("Estimating workload which takes just one millisecond");
  uint64_t nloop_per_resol = loops_per_msec() * (uint64_t)resol;
  fprintf(stdout, "End estimation; nloop_per_resol=%ld\n", nloop_per_resol);

  pid_t* pids = calloc((uint64_t)nproc, sizeof(pid_t));
  if (pids == NULL) {
    err(EXIT_FAILURE, "calloc (pids) failed: %d", errno);
  }

  struct timespec* logbuf = calloc((uint64_t)nrecord, sizeof(struct timespec));
  if (logbuf == NULL) {
    err(EXIT_FAILURE, "calloc (logbuf) failed: %d", errno);
  }

  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t ncreated = 0;
  for (uint64_t i = 0; i < (uint64_t)nproc; i++) {
    pid_t pid = fork();
    pids[i] = pid;
    if (pid < 0) {
      goto kill_children;
    }

    ncreated++;

    if (pid == 0) {
      child_fn(i, logbuf, (uint64_t)nrecord, nloop_per_resol, start);
    }
  }
  goto wait_children;

kill_children:
  for (uint64_t i = 0; i < ncreated; i++) {
    pid_t pid = pids[i];
    if (kill(pid, SIGINT) < 0) {
      warn("kill(%d) failed: %d", pid, errno);
    }
  }

wait_children:
  for (uint64_t i = 0; i < ncreated; i++) {
    if (wait(NULL) < 0) {
      warn("wait() failed: %d", errno);
    }
  }

  free(pids);
  free(logbuf);
  return 0;
}
