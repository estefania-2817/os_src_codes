#define _GNU_SOURCE
#include "../include/functions.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h> // this is for setpriority, getpriority
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

//--- Helper Function Definitions (Provided to Students) ---

/**
 * @brief Sets the CPU affinity for the calling thread.
 * @param cpu_num The CPU core number (e.g., 0, 1, 2).
 * @return 0 on success, -1 on failure.
 */
int SetCpuAffinity(int cpu_num) {
  cpu_set_t cpuset;

  // 1. Clear the CPU set structure (cpuset)
  // STUDENT CODE HERE: CPU_ZERO(...)
  CPU_ZERO(&cpuset);

  // 2. Set the bit for the desired CPU core
  // STUDENT CODE HERE: CPU_SET(...)
  CPU_SET(cpu_num, &cpuset);

  // 3. Apply the CPU affinity mask to the calling thread (pthread_self())
  // STUDENT CODE HERE: if (pthread_setaffinity_np(...) == 0) return 0; else
  // return -1;
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0) {
    return 0;
  } else {
    return -1;
  }
}

/**
 * @brief Sets the nice value (priority) for the calling process.
 * @param nice_val The nice value (-20 to 19).
 * @return 0 on success, -1 on failure (requires proper permissions).
 */
int SetNiceValue(int nice_val) {
  // We use PRIO_PROCESS and PID 0 (current process)

  // STUDENT CODE HERE: Implement the setpriority() system call.
  // if (setpriority(...) == 0) return 0; else return -1;
  if (setpriority(PRIO_PROCESS, 0, nice_val) == 0) {
    return 0;
  } else {
    return -1;
  }
}

// --- Heavy CPU-Bound Task: Matrix Multiplication ---

/**
 * @brief Performs a heavy, CPU-bound matrix multiplication (C = A * B).
 * @param arg A pointer to the nice value (passed from main).
 * @return NULL.
 */
void *HeavyCpuTask(void *arg) {
  int nice_val = *((int *)arg);
  struct timespec start, end;

  // Allocate memory for matrices A, B, and result C
  double(*A)[MATRIX_SIZE] = malloc(sizeof(double[MATRIX_SIZE][MATRIX_SIZE]));
  double(*B)[MATRIX_SIZE] = malloc(sizeof(double[MATRIX_SIZE][MATRIX_SIZE]));
  double(*C)[MATRIX_SIZE] = malloc(sizeof(double[MATRIX_SIZE][MATRIX_SIZE]));

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
  double time_taken =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

  // Get final CPU core ID (for confirmation)
  int final_cpu = sched_getcpu();

  // Calculate a simple checksum (to ensure computation wasn't optimized away)
  double checksum = C[MATRIX_SIZE / 2][MATRIX_SIZE / 2];

  // --- STUDENT LOG OUTPUT (REQUIRED FORMAT) ---
  // The student must print the final result log in the exact format.
  printf("LOG|NICE=%d|CPU=%d|TIME=%.6f|CHECKSUM=%.2f\n", nice_val, final_cpu,
         time_taken, checksum);

  // Give test script time to check process state
  usleep(500000); // 0.5 seconds

  // Clean up
  free(A);
  free(B);
  free(C);

  return NULL;
}
