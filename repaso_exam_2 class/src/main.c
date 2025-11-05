#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define BLOCK_SIZE 4096
int* first_block;

void* ModifyElement(void* param) {
  int param_int = *((int*)param);
  while (1) {
    first_block[0] += (param_int);
    printf("new value %d\n", first_block[0]);
    usleep(800000);
  }
}

int main() {
  first_block = mmap((void*)0x35000, BLOCK_SIZE, PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  printf("la direccion es %lx\n", first_block);
  pthread_t th1, th2;
  int param1 = 1;
  int param2 = -1;
  pthread_create(&th1, NULL, ModifyElement, (void*)(&param1));
  pthread_create(&th2, NULL, ModifyElement, (void*)(&param2));

  pthread_join(th1, NULL);
  pthread_join(th2, NULL);

  return 0;
}