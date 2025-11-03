#define _POSIX_C_SOURCE 200112L // action and CLOCK_MONOTONIC undefined
#include <math.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

void TimerHandlerWithSignals(int x); //declare time handler function

#define N_SAMPLES 400
#define PERIOD_MS 50 //period between samples in ms

int counter = 0;

int main(int argc, char* argv[]) {
  /// configura la senal
  struct sigaction action;
  action.sa_handler = TimerHandlerWithSignals;
  action.sa_flags = 0;
  sigaction(SIGALRM, &action, NULL); //confifure sig handleing for SIGALRM

  /// Se crea el timer
  timer_t timer_id;
  timer_create(CLOCK_MONOTONIC, NULL, &timer_id); //create timer using monotonic clock

  /*Se configuran los parámetros del timer*/
  struct itimerspec timerParams;
  /*Primer disparo*/
  timerParams.it_value.tv_sec = 0;
  timerParams.it_value.tv_nsec = PERIOD_MS * 1000000; // convertir de ms a ns
  /*Como es diferente de cero el timer es periódico*/
  timerParams.it_interval.tv_sec = 0;
  timerParams.it_interval.tv_nsec = PERIOD_MS * 1000000; // convertir de ms a ns  

  /*Se arranca el timer (tiempo relativo)*/
  timer_settime(timer_id, 0, &timerParams, NULL);   /// for timehandler function method

  struct timespec start;
  struct timespec end;

  clock_gettime(CLOCK_MONOTONIC, &start); //get time stamp using monotonic clock

  while (counter < N_SAMPLES) {    /// for timehandler function method
    pause();
    printf("%d\n", counter);
  }

              // struct timespec prev_chrono_timer;  // the code uses usleep() to pause execution for 50ms, not timer
              // while (counter < N_SAMPLES) {
              //   usleep(50000);

              //   if (counter == 0) {  // On first iteration, records the initial timestamp.
              //     clock_gettime(CLOCK_MONOTONIC, &prev_chrono_timer);
              //   }

              //   struct timespec current_chrono_timer; // Gets current timestamp on each iteration
              //   clock_gettime(CLOCK_MONOTONIC, &current_chrono_timer);

              //   double elapsed_seconds_timer = // Calculates time elapsed since previous iteration in seconds.
              //       (current_chrono_timer.tv_sec - prev_chrono_timer.tv_sec) +
              //       (current_chrono_timer.tv_nsec - prev_chrono_timer.tv_nsec) / 1E9;

              //   char buf[64];  // Formats and writes the counter and elapsed time to stdout.
              //   snprintf(buf, sizeof(buf), "%d %f\n", counter, elapsed_seconds_timer);

              //   write(1, buf, strlen(buf)); ////////////////////////////////////////////////////////////////////

              //   prev_chrono_timer = current_chrono_timer;
              //   counter++; //  Updates previous timestamp and increments counter.
              // }

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;

  printf("Elapsed time: %.9f seconds. Counter = %d \n", elapsed, counter);

  return (0); // Calculates and prints total elapsed time for all 400 samples.
}

// void TimerHandlerWithSignals(int x) {
//   counter++;
//   char buf[64];
//   snprintf(buf, sizeof(buf), "%d\n", counter);

//   write(1, buf, strlen(buf));
// }

void TimerHandlerWithSignals(int x) { // This is a signal handler function that gets called when the timer expires
  // int x parameter is the signal number (SIGALRM in this case)
  static struct timespec prev_chrono_timer; // store the previous timestamp

  if (counter == 0) { //  initialize the previous timestamp
    clock_gettime(CLOCK_MONOTONIC, &prev_chrono_timer);
  } // gets the current time from a monotonic clock

  struct timespec current_chrono_timer; // store the current timestamp
  clock_gettime(CLOCK_MONOTONIC, &current_chrono_timer); // Gets the current time and stores it in current_chrono_timer 

  double elapsed_seconds_timer = // Calculates the time elapsed since the previous timer interrupt
      (current_chrono_timer.tv_sec - prev_chrono_timer.tv_sec) +
      (current_chrono_timer.tv_nsec - prev_chrono_timer.tv_nsec) / 1E9;

  char buf[64];
  snprintf(buf, sizeof(buf), "%d %f\n", counter, elapsed_seconds_timer);
  write(1, buf, strlen(buf)); // 0:stdin, 1:stdout, 2:stderr

  prev_chrono_timer = current_chrono_timer;
  counter++;
}
