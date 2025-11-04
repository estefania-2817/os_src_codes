#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define RANGE_MAX 1000000
#define NUM_THREADS 2

long global_sum = 0;
long global_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// check prime
bool IsPrime(int n) {
  if (n <= 1) return false;
  if (n <= 3) return true;
  if (n % 2 == 0 || n % 3 == 0) return false;

  for (int i = 5; i * i <= n; i += 6) {
    if (n % i == 0 || n % (i + 2) == 0) {
      return false;
    }
  }
  return true;
}

void* calculate_primes_mutex(void* arg) {   
  int thread_id = *(int*)arg; //thread id from func argument/parameter
  int start = (thread_id * RANGE_MAX / NUM_THREADS) + 1;  //calc start and end range for thread
  int end = ((thread_id + 1) * RANGE_MAX / NUM_THREADS); // to be able to divide work evenly between threads 1 and 2

  for (int i = start; i <= end; i++) {  // T1 start: (0)*1,000,000/2 = 0 +1 = 1
    if (IsPrime(i)) {                   // T1 end: (1)*1,000,000/2 = 500,000
      pthread_mutex_lock(&mutex);       // T2 start: (1)*1,000,000/2 = 500,000 +1 = 500,001
      global_sum += i;                  // T2 end: (2)*1,000,000/2 = 1,000,000
      global_count++;
      pthread_mutex_unlock(&mutex); // lock mutex before accessing shared variables then unlock
    }
  }

  pthread_exit(NULL);
}

int main() {
  pthread_t threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];

  // create threads
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_ids[i] = i;
    if (pthread_create(&threads[i], NULL, calculate_primes_mutex, // function that the thread is gonna do
                       &thread_ids[i]) != 0) {                    // is calc_prime_mutex
      perror("Failed to create thread");
      return 1;
    }
  }

  // wait threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  //that last for loop, if num_threads =2, you can do each individually without forloop
      //pthread_join(threads[0], NULL); //Thread 0
      //pthread_join(threads[1], NULL); //Thread 1

  // destroy mutex
  pthread_mutex_destroy(&mutex);

  // print results
  printf("=== Result ===\n");
  printf("%ld\n", global_count);
  printf("%ld\n", global_sum);

  return 0;
}