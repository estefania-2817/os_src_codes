#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define SHM_NAME "/tickets_memory"
#define MAX_TICKETS 100
#define NUM_BUYERS 3

typedef struct {
  pthread_mutex_t mutex; //synchronization 
  int available_tickets;
  int transactions;
  int purchase_log[MAX_TICKETS];
} shared_data;

shared_data *data;

void *BuyTickets(void *arg) {
  int buyer_id = *(int *)arg;
  free(arg);

  // delay to ensure all threads are counted by the test
  //  first test would fail otherwise
  usleep(200000); //help with detection of threads

  while (1) {
    int tickets_wanted = rand() % 5 + 1;

    pthread_mutex_lock(&data->mutex); //LOCK before accessing shared data

    if (data->available_tickets <= 0) {
      pthread_mutex_unlock(&data->mutex);
      break;
    }
    
    // Calculate actual purchase (may be less than requested)
    int to_buy = (tickets_wanted <= data->available_tickets) //condition ? value_if_true : value_if_false
                     ? tickets_wanted
                     : data->available_tickets; 
    // If requested more than available, buy all remaining

    printf("Buyer %d requests %d tickets\n", buyer_id, tickets_wanted);
    data->available_tickets -= to_buy;
    data->purchase_log[data->transactions] = to_buy; //log purchase
    data->transactions++;

    printf("Thread %lu purchased %d tickets. Available: %d\n", pthread_self(),
           to_buy, data->available_tickets);

    pthread_mutex_unlock(&data->mutex);

    if (data->available_tickets <= 0) break;

    usleep(100000);  // 100 ms delay
  }

  pthread_exit(NULL);
}

int main() {
  int shm_fd;
  srand(time(NULL));

  // Open shared memory
  shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);

  //preferred addr, mapping length, read & write, map type-shared, file descriptor-fd, offset-where in file start mapping
  data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED,
              shm_fd, 0);

  printf("Buy your tickets right now ...\n");

  pthread_t threads[NUM_BUYERS];
  for (int i = 0; i < NUM_BUYERS; i++) {
    int *id = malloc(sizeof(int));
    *id = i + 1;
    pthread_create(&threads[i], NULL, BuyTickets, id);
  }

  usleep(500000); // After thread creation - ensures all start
  // delay after thread creation to ensure all threads start
  // because first test would fail otherwise

  for (int i = 0; i < NUM_BUYERS; i++) {
    pthread_join(threads[i], NULL);
  }

  munmap(data, sizeof(shared_data));
  close(shm_fd);

  return 0;
}
