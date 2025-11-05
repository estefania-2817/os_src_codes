#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int* first_block;
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // i added

void* modifyArray(void* param) {
  int* param_int = (int*)param;

  while (1) {
    // pthread_mutex_lock(&mutex); // i added
    if (*param_int == 1) { // T1 increments with 0.5 sec delay
      first_block[0]++; 
      usleep(500000); //remove for mutex method
    }
    if (*param_int == 2) { // T2 decrements with 0.7 sec delay
      first_block[0]--;
      usleep(700000);  //remove for mutex method
    }
    printf("[thread %d]: value = %d\n", *param_int, first_block[0]);
    // pthread_mutex_unlock(&mutex); // i added

    // usleep(*param_int == 1 ? 500000 : 700000); // mutex method
  }
}

int main() {
  first_block = mmap((void*)(0x35000), 4097, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  int* second_block = mmap((void*)(0x35000), 16000, PROT_READ | PROT_WRITE, // this second mapping will overlap with the first, replaces the first
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);    // now first map is invalid

  printf("the address of the second block is: %lx\n", second_block);
  if (second_block == MAP_FAILED) {
    printf("the second block failed\n");
  }

  pthread_t thread1;
  pthread_t thread2;
  int param1 = 1;
  int param2 = 2;

  first_block[0] = 50;
  // second_block[0] = 100; // i added
  // printf("initial value first: %d\n", first_block[0]); //i added
  // printf("initial value second: %d\n", second_block[0]); //i added //same address, values to second block change block 1

  int create = pthread_create(&thread1, NULL, modifyArray, (void*)(&param1));
  int create2 = pthread_create(&thread2, NULL, modifyArray, (void*)(&param2));
  if (create == -1 || create2 == -1) {
    perror("error pthreade create: ");
  }

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  while (1) {
    sleep(1);
  }

  return 0;
}