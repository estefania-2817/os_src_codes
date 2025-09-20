#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main() {
  // creamos el queue
  struct mq_attr attr;

  // Initialize the queue attributes
  attr.mq_flags = 0;
  attr.mq_maxmsg = 5;
  attr.mq_msgsize = 8192;
  attr.mq_curmsgs = 0;

  mqd_t queue_os;
  queue_os = mq_open("/robert_queue", O_CREAT | O_WRONLY, 0644, &attr);
  if (queue_os == -1) {
    perror("error in open");
    exit(EXIT_FAILURE);
  }

  // inicializamos el mensaje
  char message[8192];

  // evniar el mensaje
  for (int i = 0; i < 10; i++) {
    snprintf(message, 8192, "mensaje numero %d", i);
    mq_send(queue_os, message, 8192, 0);
    printf("acabo de evniar el mensaje %s\n", message);
  }

  return 0;
}