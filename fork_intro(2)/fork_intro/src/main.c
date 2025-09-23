#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main()
{
  pid_t pid;
  pid_t pid2;

  pid = getpid();

  printf("my pid is: %d\n", pid); // parent ID

  pid2 = fork(); // create child process

  if (pid2 == 0)
  {
    pid = getpid();
    printf("hello, i am the son. I just got here. my pid is: %d\n", pid);
  }
  else
  {
    pid = getpid();
    printf("hello, i am the father. my pid is: %d\n", pid);
  }

  printf("hola que tal\n");

  return 0;
}

// my pid is: 2667
// hello, i am the father. my pid is: 2667
// hola que tal
// hello, i am the son. I just got here. my pid is: 2668
// hola que tal
