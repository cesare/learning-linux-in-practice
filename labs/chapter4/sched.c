#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <strings.h>
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

static void kill_all_children(pid_t* pids, uint64_t ncreated) {
  for (uint64_t i = 0; i < ncreated; i++) {
    pid_t pid = pids[i];
    if (kill(pid, SIGINT) < 0) {
      warn("kill(%d) failed", pid);
    }
  }
}

static void wait_all_children(pid_t* pids, uint64_t ncreated) {
  for (uint64_t i = 0; i < ncreated; i++) {
    pid_t pid = pids[i];
    if (waitpid(pid, NULL, 0) < 0) {
      warn("waitpid() failed");
    }
  }
}

typedef struct {
  int32_t index_for_rest_args;
  int8_t pad[2];
  bool help;
  bool verbose;
} Options;

typedef struct {
  uint64_t nproc;
  uint64_t resol;
  uint64_t nrecord;
  int8_t pad[6];
  bool help;
  bool verbose;
} Config;

static void show_usage(char** argv) {
  fprintf(stderr, "Usage: %s [options] <nproc> <total[ms]> <resolution[ms]>\n", argv[0]);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --verbose=(true|false)\n");
  fprintf(stderr, "  --help\n");
}

static bool parse_boolean_value(const char* str, bool default_value) {
  if (str == NULL) {
    return default_value;
  }

  /*
   * patterns for true
   */
  if (strncasecmp(str, "true", 4) == 0) {
    return true;
  }
  if (strncasecmp(str, "yes", 3) == 0) {
    return true;
  }
  if (strncmp(str, "1", 1) == 0) {
    return true;
  }

  /*
   * patterns for false
   */
   if (strncasecmp(str, "false", 5) == 0) {
     return false;
   }
   if (strncasecmp(str, "no", 2) == 0) {
     return false;
   }
   if (strncmp(str, "0", 1) == 0) {
     return false;
   }

   return default_value;
}

static void parse_options(int argc, char** argv, Options* options) {
  struct option option_defs[] = {
    {"help",          no_argument, NULL, 'h'},
    {"verbose", optional_argument, NULL, 'v'},
    {0, 0, 0, 0}
  };

  while (true) {
    int32_t c = getopt_long(argc, argv, "hv+", option_defs, NULL);
    if (c == -1) {
      break;
    }

    switch (c) {
      case 'h':
        options->help = true;
        break;
      case 'v':
        options->verbose = parse_boolean_value(optarg, true);
        break;
      default:
        show_usage(argv);
        exit(EXIT_FAILURE);
    }
  }

  options->index_for_rest_args = optind;  // optind is a global variable
}

static Config* parse_arguments(int argc, char** argv) {
  Options options = {.verbose = false, .help = false};
  parse_options(argc, argv, &options);

  int32_t offset = options.index_for_rest_args;
  if (argc - offset != 3) {
    show_usage(argv);
    exit(EXIT_FAILURE);
  }

  int64_t nproc_parsed = strtol(argv[offset + 0], NULL, 10);
  int64_t total_parsed = strtol(argv[offset + 1], NULL, 10);
  int64_t resol_parsed = strtol(argv[offset + 2], NULL, 10);

  if (nproc_parsed < 1) {
    fprintf(stderr, "<nproc>(%ld) should be >= 1\n", nproc_parsed);
    exit(EXIT_FAILURE);
  }

  if (total_parsed < 1) {
    fprintf(stderr, "<total>(%ld) should be >= 1\n", total_parsed);
    exit(EXIT_FAILURE);
  }

  if (resol_parsed < 1) {
    fprintf(stderr, "<resol>(%ld) should be >= 1\n", resol_parsed);
    exit(EXIT_FAILURE);
  }

  uint64_t nproc = (uint64_t)nproc_parsed;
  uint64_t total = (uint64_t)total_parsed;
  uint64_t resol = (uint64_t)resol_parsed;

  if (total % resol != 0) {
    fprintf(stderr, "<total>(%ld) should be multiple of <resolution>(%ld)\n", total, resol);
    exit(EXIT_FAILURE);
  }

  Config* config = calloc(1, sizeof(Config));
  if (config == NULL) {
    err(EXIT_FAILURE, "calloc (Config) failed");
  }
  config->nproc   = nproc;
  config->resol   = resol;
  config->nrecord = total / resol;
  config->help    = options.help;
  config->verbose = options.verbose;
  return config;
}

static uint64_t estimate_loops_per_msec(Config* config) {
  puts("Estimating workload which takes just one millisecond");
  uint64_t nloop_per_resol = loops_per_msec() * config->resol;
  fprintf(stdout, "End estimation; nloop_per_resol=%ld\n", nloop_per_resol);

  return nloop_per_resol;
}

int main(int argc, char** argv) {
  Config* config = parse_arguments(argc, argv);
  if (config->help) {
    show_usage(argv);
    exit(EXIT_SUCCESS);
  }

  uint64_t nloop_per_resol = estimate_loops_per_msec(config);

  pid_t* pids = calloc(config->nproc, sizeof(pid_t));
  if (pids == NULL) {
    err(EXIT_FAILURE, "calloc (pids) failed");
  }

  struct timespec* logbuf = calloc(config->nrecord, sizeof(struct timespec));
  if (logbuf == NULL) {
    err(EXIT_FAILURE, "calloc (logbuf) failed");
  }

  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  uint64_t ncreated = 0;
  for (uint64_t i = 0; i < config->nproc; i++) {
    pid_t pid = fork();
    pids[i] = pid;
    if (pid < 0) {
      kill_all_children(pids, ncreated);
      goto cleanup;
    }

    ncreated++;

    if (pid == 0) {
      child_fn(i, logbuf, config->nrecord, nloop_per_resol, start);
    }
  }

cleanup:
  wait_all_children(pids, ncreated);
  free(pids);
  free(logbuf);
  free(config);

  return 0;
}
