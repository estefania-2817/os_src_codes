#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main()
{
  // eliminamos queues antiguos
  struct mq_attr attr; // Declare a variable of type struct mq_attr

  // Initialize the queue attributes
  attr.mq_flags = 0;      // Flags (0 = blocking operations)
  attr.mq_maxmsg = 5;     // Maximum messages queue can hold
  attr.mq_msgsize = 8192; // Maximum size of each message (8KB)
  attr.mq_curmsgs = 0;    // Current messages in queue (initialized to 0)
  // mq_unlink("/robert_queue");

  // conectarse al queue
  mqd_t queue_os;                                                       // declare message queue descriptor                                                   // Declare message queue descriptor
  queue_os = mq_open("/robert_queue", O_CREAT | O_RDONLY, 0644, &attr); // create queue(if doesnt exist), READ only
  if (queue_os == -1)                                                   // check if error in creating queue
  {
    perror("error in open in the reader");
    exit(EXIT_FAILURE);
  }

  // leer el queue
  char buffer[8192]; // Buffer to receive message
  int priority;      // Variable to store message priority

  printf("voy a leer el mq\n");
  mq_receive(queue_os, buffer, 8192, &priority);

  printf("the received message is %s\n", buffer);

  return 0;
}