#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/shm_example"

int main() {
  // Subscribe
  int fd = shm_open(SHM_NAME, O_RDWR, 0666);
  ftruncate(fd, 1024);
  char *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  shm_ptr[4] = 'y';

  // Read data from block
  printf("data from shared memory: %s\n", shm_ptr);
  return 0;
}