#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main() {
  pid_t pid;
  pid_t pid2;

  pid = getpid();
  printf("my pid is: %d\n", pid);

  pid2 = fork();

  if (pid2 == 0) {
    pid = getpid();
    printf("hello, i am the son. I just got here. my pid is: %d\n", pid);
    // execlp("ls","ls","-l","-r",NULL);
    // execlp("./arrays_example","arrays",NULL);

    if (execlp("htop", "htop_cooler", NULL) == -1) {
      perror("execlp failed");
      exit(EXIT_FAILURE);
    }
    printf("I got back!!\n");
    return EXIT_SUCCESS;
  } else {
    pid = getpid();
    printf("hello, i am the father. my pid is: %d\n", pid);
    wait(NULL);
    printf("mi hijo ha muerto\n");
  }

  printf("hola que tal\n");

  return 0;
}
