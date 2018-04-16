#include <err.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ALLOC_SIZE (100*1024*1024)
#define BUFFER_SIZE 1000

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage %s filepath\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char overwrite_data[] = "HELLO";
  pid_t pid = getpid();

  char command[BUFFER_SIZE];
  snprintf(command, BUFFER_SIZE, "cat /proc/%d/maps", pid);

  puts("*** memory map before mapping file ***");
  fflush(stdout);

  system(command);

  /*
   * value of variable path should be a path outside of VirtualBox shared directory.
   * If you specify such as ./testfile, mmap will fail with EINVAL error.
   */
  char* path = argv[1];
  int32_t fd = open(path, O_RDWR);
  if (fd < 0) {
    err(EXIT_FAILURE, "open() failed");
  }

  void* file_contents = mmap(NULL, ALLOC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (file_contents == MAP_FAILED) {
    warn("mmap() failed");
    goto close_file;
  }

  puts("");
  fprintf(stdout, "*** succeeded to map file: address = %p; size = 0x%x ***\n", file_contents, ALLOC_SIZE);

  puts("");
  puts("*** memory map after mapping file ***");
  fflush(stdout);

  system(command);

  puts("");
  fprintf(stdout, "*** file contents before overwrite mapped region: %s\n", file_contents);

  memcpy(file_contents, overwrite_data, strlen(overwrite_data));

  puts("");
  fprintf(stdout, "*** overwritten mapped region with: %s\n", file_contents);

  if (munmap(file_contents, ALLOC_SIZE) < 0) {
    warn("munmap() failed");
  }

close_file:
  if (close(fd) < 0) {
    warn("close() failed");
  }

  return EXIT_SUCCESS;
}
