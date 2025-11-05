#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_NAME "/mem_block_exam_1"
#define MQ_NAME "/mq_exam_1"
#define NUM_VALUES 10
#define SHM_SIZE NUM_VALUES * sizeof(double)
#define MAX_MSG_SIZE 1024
#define MAX_QUEUE_SIZE 10
// Global variables
pid_t pid_creator;
pid_t pid_logger;
mqd_t mq;
double *shm_ptr;
int fd;
// int shm_fd;
int signal_count = 0;  // ADDED

// void cleanup() {
//   if (pid_creator > 0) kill(pid_creator, SIGINT);
//   if (pid_logger > 0) kill(pid_logger, SIGINT);

//   if (pid_creator > 0) waitpid(pid_creator, NULL, 0);
//   if (pid_logger > 0) waitpid(pid_logger, NULL, 0);

//   // Close message queue
//   if (mq != -1) mq_close(mq);

//   // Unmap and close shared memory
//   if (shm_ptr != MAP_FAILED) munmap(shm_ptr, SHM_SIZE);
//   if (fd != -1) {
//     close(fd);
//     shm_unlink("SHM_NAME");
//   }

//   mq_unlink("MQ_NAME");

//   exit(0);
// }

void signal_handler(int sig) {
  // ADDED necessary average calc, was testing signal before
  if (sig == SIGUSR2) {
    printf("Data Processor: new data available\n");

    // average
    double sum = 0.0;
    for (int i = 0; i < NUM_VALUES; i++) {
      sum += shm_ptr[i];
    }
    double average = sum / NUM_VALUES;

    // string
    char buffer[80];
    snprintf(buffer, sizeof(buffer), "%f", average);

    // message queue
    mq_send(mq, buffer, strlen(buffer) + 1, 0);

    signal_count++;
  }
  // else if (sig == SIGINT)
  // {
  //   printf("exiting\n");
  //   exit(0);
  // }
}

int main() {
  pid_t pid = getpid();
  char *mq_name = MQ_NAME;
  char *shm_name = SHM_NAME;

  // Setup signal handler for Ctrl+C
  // signal(SIGINT, cleanup);

  fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
  ftruncate(fd, SHM_SIZE);
  shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = MAX_QUEUE_SIZE;  // number of messages
  attr.mq_msgsize = MAX_MSG_SIZE;   // message size
  attr.mq_curmsgs = 0;

  mq = mq_open(mq_name, O_CREAT | O_RDWR, 0666, &attr);  // Create message queue

  printf("SHM and message queue created successfully\n");

  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigaction(SIGUSR2, &sa, NULL);  // handler for SIGUSR2 signal

  // sleep(2);

  pid_creator = fork();
  if (pid_creator == 0) {
    printf("CREATOR\n");
    char buffer[80];  // no lo habia cambiado a string
    snprintf(buffer, sizeof(buffer), "%d", pid);
    execlp("./data_creator", "./data_creator", shm_name, buffer, NULL);
    perror("\nexecl child failed");
  }

  pid_logger = fork();
  if (pid_logger == 0) {
    printf("LOGGER\n");
    execlp("./data_logger", "./data_logger", mq_name, NULL);
    perror("\nexecl child failed");
  }

  printf("Process creator PID: %d\n", pid_creator);
  printf("Process logger  PID: %d\n", pid_logger);

  while (signal_count < 5) {
    pause();  // wait for signals
  }

  // sleep(8);
  // kill(pid, SIGUSR2);
  // pause();
  // signal(SIGINT, cleanup);

  // Cleanup shared memory resources
  munmap(shm_ptr, 1024);
  close(fd);           // me falto esto
  mq_close(mq);        // y esto
  mq_unlink(MQ_NAME);  // y esto
  shm_unlink(SHM_NAME);
  printf("SHM block deleted\n");
  return 0;
}