#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define RANGE_MAX 1000000
#define NUM_THREADS 2

typedef struct {
  int thread_id;
  int64_t local_sum;
  int64_t local_count;
} thread_data_t;   //thread data object

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

void* calculate_primes_local(void* arg) {
  thread_data_t* data = (thread_data_t*)arg;
  int start = (data->thread_id * RANGE_MAX / NUM_THREADS) + 1;
  int end = ((data->thread_id + 1) * RANGE_MAX / NUM_THREADS);

  //initialize local counters
  data->local_sum = 0;
  data->local_count = 0;

  for (int i = start; i <= end; i++) {
    if (IsPrime(i)) {
      data->local_sum += i;
      data->local_count++;
    }
  }

  pthread_exit(NULL);
}

int main() {
  pthread_t threads[NUM_THREADS]; // array of thread identifiers
  thread_data_t thread_data[NUM_THREADS]; // array of thread data
  int64_t total_sum = 0;
  int64_t total_count = 0;

  // create threads
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_data[i].thread_id = i;
    if (pthread_create(&threads[i], NULL, calculate_primes_local, // function that the thread is gonna do
                       &thread_data[i]) != 0) {                   // is calc_prime_mutex
      perror("Failed to create thread");
      return 1;
    }
  }

  // wait threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // combine results
  for (int i = 0; i < NUM_THREADS; i++) {
    total_sum += thread_data[i].local_sum;
    total_count += thread_data[i].local_count;
  }

  // print results
  printf("=== Result ===\n");
  printf("%ld\n", total_count);
  printf("%ld\n", total_sum);

  return 0;
}

// Mutex = Safety but slower
// Local variables = Faster but combine results later
// Proper thread synchronization is crucial for correct results