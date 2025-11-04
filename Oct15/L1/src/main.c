#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "../include/functions.h"

// --- Main Program ---

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <nice_value> <cpu_number>\n", argv[0]);
    fprintf(stderr, "Example: %s 10 1\n", argv[0]);
    return 1;
  }

  int nice_val = atoi(argv[1]);
  int cpu_num = atoi(argv[2]);
  pthread_t thread;

  // Log the process start for the test script
  printf("TEST_START|PID=%d|NICE=%d|CPU=%d\n", getpid(), nice_val, cpu_num);

  // INSERT HERE THE CPU AFFINITY AND PRIORITY FUNCTIONS
  // Set process nice value
  if (SetNiceValue(nice_val) != 0) {
    fprintf(stderr, "Failed to set nice value\n");
  }

  // Set CPU affinity for main thread (which will be inherited by child thread)
  if (SetCpuAffinity(cpu_num) != 0) {
    fprintf(stderr, "Failed to set CPU affinity\n");
  }

  // Create the thread, passing the nice value as argument
  // NOTE: Remember to send pointers to a thread
  if (pthread_create(&thread, NULL, HeavyCpuTask, &nice_val) != 0) {
    fprintf(stderr, "Failed to create thread\n");
    return 1;
  }
  pthread_join(thread, NULL);

  printf("TEST_END\n");

  return 0;
}
