#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/shm_example"

int main() {
  // Create block
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  ftruncate(fd, 1024);
  char *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }
  // write data
  strcpy(shm_ptr, "Hello from Shared Memory!");
  printf("Written to shared memory: %s\n", shm_ptr);

  // Destroy block
  printf("Press ENTER to delete block\n");
  getchar();

  printf("data from shared memory: %s\n", shm_ptr);

  munmap(shm_ptr, 1024);
  shm_unlink(SHM_NAME);
  printf("SHM block deleted\n");
  return 0;
}
