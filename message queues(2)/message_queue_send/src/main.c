#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main()
{
  // creamos el queue
  struct mq_attr attr; // Declare a variable of type struct mq_attr

  // Initialize the queue attributes
  attr.mq_flags = 0;      // Flags (0 = blocking operations)
  attr.mq_maxmsg = 5;     // Maximum messages queue can hold
  attr.mq_msgsize = 8192; // Maximum size of each message (8KB)
  attr.mq_curmsgs = 0;    //

  mqd_t queue_os;                                                       // Declare message queue descriptor
  queue_os = mq_open("/robert_queue", O_CREAT | O_WRONLY, 0644, &attr); // create queue(if doesnt exist), WRITE only
  if (queue_os == -1)                                                   // check if error in creating queue
  {
    perror("error in open");
    exit(EXIT_FAILURE);
  }

  // inicializamos el mensaje de buffer
  char message[8192]; /// Buffer to hold messages (same size as mq_msgsize)

  // evniar el mensaje
  for (int i = 0; i < 10; i++)
  {
    snprintf(message, 8192, "meeensaje numero %d", i);  // Format the message into the buffer, safely
    mq_send(queue_os, message, 8192, 0);                // Send the message to the queue
    printf("acabo de evniar el mensaje %s\n", message); // confirmation
  }

  return 0;
}