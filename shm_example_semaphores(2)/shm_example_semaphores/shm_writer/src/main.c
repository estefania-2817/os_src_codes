#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/shm_example"

int main() {
  int fd = shm_open(SHM_NAME, O_RDWR, 0666); // Open existing shared memory, created by reader

  float *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        // Map shared memory into this process's address space (same parameters as first program)

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  sem_t *sem = sem_open("/mysem", 0); // Open existing semaphore (created by first program)

  for (int i = 0; i < 20; i++) {
    shm_ptr[i] = i * 0.66;
    printf("Written to shared memory: %f\n", shm_ptr[i]);
  }  // Write data to shared memory and print what was written

  sem_post(sem); // Signal semaphore - this wakes up the first program that was waiting

  return 0;
}
