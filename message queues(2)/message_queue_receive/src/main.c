#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main() {
  // eliminamos queues antiguos
  struct mq_attr attr;

  // Initialize the queue attributes
  attr.mq_flags = 0;
  attr.mq_maxmsg = 5;
  attr.mq_msgsize = 8192;
  attr.mq_curmsgs = 0;
  // mq_unlink("/robert_queue");

  // conectarse al queue
  mqd_t queue_os;
  queue_os = mq_open("/robert_queue", O_CREAT | O_RDONLY, 0644, &attr);
  if (queue_os == -1) {
    perror("error in open in the reader");
    exit(EXIT_FAILURE);
  }

  // leer el queue
  char buffer[8192];
  int priority;

  printf("voy a leer el mq\n");
  mq_receive(queue_os, buffer, 8192, &priority);

  printf("the received message is %s\n", buffer);

  return 0;
}