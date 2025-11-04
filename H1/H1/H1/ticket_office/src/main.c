#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/tickets_memory"
#define MAX_TICKETS 100

typedef struct {
  pthread_mutex_t mutex; //synchronization
  int available_tickets;
  int transactions;
  int purchase_log[MAX_TICKETS];
} shared_data;

int main() {
  int shm_fd;
  shared_data *data;

  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

  ftruncate(shm_fd, sizeof(shared_data));

  //preferred addr, mapping length, read & write, map type-shared, file descriptor-fd, offset-where in file start mapping
  data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED,
              shm_fd, 0);

  // shared memory mutex initialization
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); // allows mutex to work across processes
  pthread_mutex_init(&data->mutex, &attr); //Initialize the mutex in shared memory with process-shared attribute

  data->available_tickets = 100;
  data->transactions = 0;
  memset(data->purchase_log, 0, sizeof(data->purchase_log)); // Initialize all purchase log entries to 0

  printf("TICKET REPORT:\nAvailable tickets: %d\n\n", data->available_tickets);

  // print until sold out
  while (1) {
    sleep(1);

    pthread_mutex_lock(&data->mutex); // Lock mutex before accessing shared data
    printf("TICKET REPORT:\n");
    printf("Available tickets: %d\n", data->available_tickets);

    if (data->available_tickets == 0) {
      printf("SOLD OUT...\n");
      pthread_mutex_unlock(&data->mutex);
      break;
    }

    pthread_mutex_unlock(&data->mutex);
    printf("\n");
  }

  // cleanup
  munmap(data, sizeof(shared_data));
  close(shm_fd);
  shm_unlink(SHM_NAME);

  return 0;
}
