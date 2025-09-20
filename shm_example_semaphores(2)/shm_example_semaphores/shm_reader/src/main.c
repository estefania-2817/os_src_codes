#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#define SHM_NAME "/shm_example"

int main() {
  shm_unlink(SHM_NAME);
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  ftruncate(fd, 1024);
  float *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  sem_unlink("/mysem");
  sem_t *sem = sem_open("/mysem", O_CREAT, 0666, 0);

  printf("Semaphore created. I'm going to wait now...\n");
  sem_wait(sem);

  for (int i = 0; i < 20; i++) {
    printf("Read from shared memory: %f\n", shm_ptr[i]);
  }

  munmap(shm_ptr, 1024);
  shm_unlink(SHM_NAME);
  printf("SHM block deleted\n");

  return 0;
}
