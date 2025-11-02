#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#define SHM_NAME "/shm_example"

int main() {
  shm_unlink(SHM_NAME); // Remove existing shared memory with the given name to start fresh

  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666); // create shared memory
  ftruncate(fd, 1024); // Set shared memory size to 1024 bytes

  float *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      //   0 - let OS choose address
      // 1024 - size to map
      // PROT_READ | PROT_WRITE - read/write permissions
      // MAP_SHARED - changes are visible to other processes
      // fd - file descriptor of shared memory
      // 0 - offset
  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  sem_unlink("/mysem"); // Remove existing semaphore to start fresh
  sem_t *sem = sem_open("/mysem", O_CREAT, 0666, 0); // 0 - initial semaphore value (locked)

  printf("Semaphore created. I'm going to wait now...\n");
  sem_wait(sem); // WAITT for semaphore - this blocks until the other process calls sem_post()

  for (int i = 0; i < 20; i++) {
    printf("Read from shared memory: %f\n", shm_ptr[i]);
  } // Read and print 20 float values from shared memory

  munmap(shm_ptr, 1024);
  shm_unlink(SHM_NAME);
  printf("SHM block deleted\n");

  return 0;
}
