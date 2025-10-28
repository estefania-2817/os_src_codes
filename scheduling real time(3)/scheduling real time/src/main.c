#define _GNU_SOURCE

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

void* HeavyCpuTask(void* arg);

#define MATRIX_SIZE 300
#define NUM_ITERATIONS 500

int counter = 0;

int main() {
  srand(time(NULL));

  pthread_t thread1, thread2;

  // Create the thread, passing the nice value as argument
  int ids[2] = {1, 2};

  pthread_attr_t attr;
  struct sched_param param;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  param.sched_priority = 16;
  pthread_attr_setschedparam(&attr, &param);

  pthread_create(&thread1, &attr, HeavyCpuTask, &ids[0]);
  printf("Created thread 1\n");

  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  param.sched_priority = 16;
  pthread_attr_setschedparam(&attr, &param);

  pthread_create(&thread2, &attr, HeavyCpuTask, &ids[1]);
  printf("Created thread 2\n");

  // Wait for the thread to complete
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  return 0;
}

/**
 * @brief Performs a heavy, CPU-bound matrix multiplication (C = A * B).
 * @param arg A pointer to the nice value (passed from main).
 * @return NULL.
 */
void* HeavyCpuTask(void* arg) {
  struct timespec start, end;

  int* id = (int*)arg;
  float total_time = 0.0;

  if (*id == 1) {
    // Set CPU
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(8, &cpuset);
    CPU_SET(13, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // Set nice value to 0
    // if (setpriority(PRIO_PROCESS, 0, 10) != 0) {
    //   fprintf(stderr, "Thread %d: Failed to set nice value: %s\n", *id,
    //           strerror(errno));
    // }
  } else {
    // Set CPU
    // cpu_set_t cpuset;
    // CPU_ZERO(&cpuset);
    // CPU_SET(8, &cpuset);
    // pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // Set nice value to 19
    // if (setpriority(PRIO_PROCESS, 0, 15) != 0) {
    //   fprintf(stderr, "Thread %d: Failed to set nice value: %s\n", *id,
    //           strerror(errno));
    // }
  }

  // Allocate memory for matrices A, B, and result C
  double (*A)[MATRIX_SIZE] = malloc(sizeof(double[MATRIX_SIZE][MATRIX_SIZE]));
  double (*B)[MATRIX_SIZE] = malloc(sizeof(double[MATRIX_SIZE][MATRIX_SIZE]));
  double (*C)[MATRIX_SIZE] = malloc(sizeof(double[MATRIX_SIZE][MATRIX_SIZE]));

  if (!A || !B || !C) {
    fprintf(stderr, "ERROR: Memory allocation failed for matrices.\n");
    return NULL;
  }

  // Initialize matrices A and B
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      A[i][j] = (double)(i + j) * 0.1;
      B[i][j] = (double)(i - j) * 0.1;
      C[i][j] = 0.0;
    }
  }

  for (int m = 0; m < NUM_ITERATIONS; m++) {
    // --- STUDENT MUST START TIME MEASUREMENT HERE ---
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Matrix Multiplication (C = A * B)
    for (int i = 0; i < MATRIX_SIZE; i++) {
      for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
          C[i][j] += A[i][k] * B[k][j];
        }
      }
    }

    // --- STUDENT MUST END TIME MEASUREMENT HERE ---
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate time taken in seconds
    double time_taken = (double)(end.tv_sec - start.tv_sec) +
                        (double)(end.tv_nsec - start.tv_nsec) / 1e9;
    total_time += time_taken;

    // Get final CPU core ID (for confirmation)
    int final_cpu = sched_getcpu();

    // Calculate a simple checksum (to ensure computation wasn't optimized away)
    double checksum = C[MATRIX_SIZE / 2][MATRIX_SIZE / 2];

    // --- STUDENT LOG OUTPUT (REQUIRED FORMAT) ---
    // The student must print the final result log in this exact format.
    printf("[thread %d]: multiplication %d, time taken: %f\n", *id, m,
           time_taken);
  }

  printf("[THREAD %d]: ENDED, TOTAL TIME: %f\n", *id, total_time);

  // Clean up
  free(A);
  free(B);
  free(C);

  return NULL;
}
