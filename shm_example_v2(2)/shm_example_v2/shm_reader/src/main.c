#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/shm_example"
#define N_ELEM 45 // number of elements in array
#define BLOCK_SIZE N_ELEM * sizeof(float) // total memory size needed

int main() {
  // Subscribe
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666); // create and open shared memory
  ftruncate(fd, BLOCK_SIZE); // set shared memory size
  float *shm_ptr =
      mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      // Maps the shared memory into this process's address space

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  // write data
  for (int i = 0; i < N_ELEM; i++) {
    printf("%f\n", shm_ptr[i]); // Prints all values from the shared memory array
  }
  printf("\n");

  munmap(shm_ptr, BLOCK_SIZE);
  shm_unlink(SHM_NAME);
  printf("SHM block deleted\n");

  return 0;
}