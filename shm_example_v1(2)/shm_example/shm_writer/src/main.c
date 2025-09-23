#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/shm_example"  // name of shared memory object

int main() {
  // Create new shared memory object/block
  // O_CREAT = create if doesn't exist, O_RDWR = read-write access
  // 0666 = read/write permissions for owner, group, others
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  ftruncate(fd, 1024);  // set size of shared memory object
  // Map shared memory into process's address space
  char *shm_ptr = mmap(0, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  // Creates memory mapping: 0=let OS choose address, 1024=size,
  // PROT_READ|PROT_WRITE=read/write permissions, MAP_SHARED=changes are visible
  // to other processes

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }
  // write data to shared memory block
  strcpy(shm_ptr, "Hello from Shared Memory!");
  printf("Written to shared memory: %s\n", shm_ptr);

  // Destroy block
  printf("Press ENTER to delete block\n");
  getchar();  // pause until user presses ENTER

  printf("data from shared memory: %s\n", shm_ptr);

  // Cleanup shared memory resources
  munmap(shm_ptr, 1024);  // unmap shared memory from process's address space
  shm_unlink(SHM_NAME);   // Mark shared memory object for deletion
  printf("SHM block deleted\n");
  return 0;
}
