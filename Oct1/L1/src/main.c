
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_HORSES 5
#define TRACK_LENGTH 50

int track[NUM_HORSES] = {0};
int finish_order[NUM_HORSES];
int finish_count = 0;  // count horses that finished

void *horse_run(void *arg) {
  int horse_id = *((int *)arg);
  free(arg);  // free alloc mem for ID

  while (1) {
    // random steps 0-3
    int steps = rand() % 4;
    track[horse_id] += steps;

    // not exceed track length
    if (track[horse_id] > TRACK_LENGTH) {
      track[horse_id] = TRACK_LENGTH;
    }

    printf("Horse %d moves to %d\n", horse_id, track[horse_id]);

    // check if finished
    if (track[horse_id] >= TRACK_LENGTH) {
      printf("Horse %d finished the race!\n", horse_id);

      finish_order[finish_count] = horse_id;
      finish_count++;

      break;
    }

    // delay, gallop
    usleep(10000 + (rand() % 20000));  // 10-30ms delay
  }

  return 0;
}

int main() {
  pthread_t horses[NUM_HORSES];

  // seed rand num
  srand(time(NULL));

  printf("Start race\n");
  printf("Track length: %d\n\n", TRACK_LENGTH);

  // horse threads
  for (int i = 0; i < NUM_HORSES; i++) {
    // creates unique memory for each thread's ID to avoid race conditions
    int *horse_id = malloc(sizeof(int));
    *horse_id = i;

    // horses[i] - store the thread identifier
    // NULL default attributes
    pthread_create(&horses[i], NULL, horse_run, horse_id);
  }

  // wait for threads to finish
  for (int i = 0; i < NUM_HORSES; i++) {
    pthread_join(horses[i], NULL);
  }

  // print results
  printf("\n--- Final results ---\n");
  for (int i = 0; i < NUM_HORSES; i++) {
    printf("Place %d: Horse %d\n", i + 1, finish_order[i]);
  }

  return 0;
}