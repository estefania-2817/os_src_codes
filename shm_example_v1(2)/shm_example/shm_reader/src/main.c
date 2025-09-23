#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/shm_example"  // name of shared memory object

int main() {
  // subscribe to existing shared memory
  int fd = shm_open(SHM_NAME, O_RDWR, 0666);
  ftruncate(fd, 1024);  // set size of shared memory object
  // Map shared memory into process's address space
  char *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shm_ptr == MAP_FAILED) {  // check for mapping error, fail
    perror("mmap failed");
    exit(1);
  }

  shm_ptr[4] = 'y';  // modify 5th character of shared memory ('o' to 'y')

  // Read data from shared memory block
  printf("data from shared memory: %s\n", shm_ptr);
  return 0;
}