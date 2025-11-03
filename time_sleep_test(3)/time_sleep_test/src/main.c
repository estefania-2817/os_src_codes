#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void TenSecondSleep1() {
  float y = 0;
  for (int i = 0; i < 10; i++) {
    sleep(1);
  }
}

void TenSecondSleep2() {
  float y = 0;
  for (int i = 0; i < 1000; i++) {
    usleep(10000);
  }
}

void TenSecondSleep3() {
  struct timespec time_data;

  time_data.tv_sec = 0;
  time_data.tv_nsec = 10000000;

  for (int i = 0; i < 1000; i++) {
    nanosleep(&time_data, NULL);
  }
}

int main() {
  struct timespec start;
  struct timespec end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  TenSecondSleep3();
  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;

  printf("Elapsed time: %.9f seconds\n", elapsed);

  // cuanto tiempo de CPU se invirtio aca?
  struct timespec end_cpu_time;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_cpu_time);
  double elapsed_cpu = end_cpu_time.tv_sec + end_cpu_time.tv_nsec / 1E9;
  printf("Actual CPU time: %.9f seconds\n", elapsed_cpu);

  return 0;
}
