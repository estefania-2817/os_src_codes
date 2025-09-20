#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/shm_example"

int main() {
  int fd = shm_open(SHM_NAME, O_RDWR, 0666);
  // ftruncate(fd, 1024);
  float *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  sem_t *sem = sem_open("/mysem", 0);

  for (int i = 0; i < 20; i++) {
    shm_ptr[i] = i * 0.66;
    printf("Written to shared memory: %f\n", shm_ptr[i]);
  }

  sem_post(sem);

  return 0;
}
