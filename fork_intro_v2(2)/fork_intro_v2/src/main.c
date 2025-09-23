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
  printf("my pid is: %d\n", pid);

  pid2 = fork();

  // both processes run at the same time from here, but whichever can print
  // first will do it first
  if (pid2 == 0)
  {
    // child process code
    pid = getpid();
    printf("hello, i am the son. I just got here. my pid is: %d\n", pid);
    // execlp("ls", "ls", "-l", "-r", NULL);
    // execlp("./arrays_example", "arrays", NULL);

    // Attempt to REPLACE the child process with "htop"
    if (execlp("htop", "htop_cooler", NULL) == -1)
    {
      printf("hello error.\n");
      perror("execlp failed");
      exit(EXIT_FAILURE);
    }

    // ***THIS LINE NEVER EXECUTES IF execlp SUCCEEDS***
    printf("I got back!!\n");
    return EXIT_SUCCESS;
  }
  else
  {
    // PARENT PROCESS CODE
    pid = getpid();
    printf("hello, i am the father. my pid is: %d\n", pid);
    wait(NULL);
    printf("mi hijo ha muerto\n");
  }

  printf("hola que tal\n");

  return 0;
}

// WITHOUT the ./array_example
//  my pid is: 8266
//  hello, i am the father. my pid is: 8266
//  hello, i am the son. I just got here. my pid is: 8267
//  execlp failed: No such file or directory
//  mi hijo ha muerto
//  hola que tal

// WITH the ./array_example
//  my pid is: 9259
//  hello, i am the father. my pid is: 9259
//  hello, i am the son. I just got here. my pid is: 9260
//  type a text: hello estefania
//  You typed: hello estefania

// The new version is: HeLlO EsTeFaNiA

// mi hijo ha muerto
// hola que tal

// WITH the ls
// hello, i am the father. my pid is: 10357
// hello, i am the son. I just got here. my pid is: 10359
// total 60
// -rwxrwxrwx 1 estefania2817 estefania2817 16256 Sep 22 10:48 fork_intro
// -rwxrwxrwx 1 estefania2817 estefania2817  1744 Sep 22 10:35 cmake_install.cmake
// -rwxrwxrwx 1 estefania2817 estefania2817 16256 Sep 20 17:38 arrays_example
// -rwxrwxrwx 1 estefania2817 estefania2817  5537 Sep 22 10:35 Makefile
// drwxrwxrwx 1 estefania2817 estefania2817  4096 Sep 22 10:48 CMakeFiles
// -rwxrwxrwx 1 estefania2817 estefania2817 14599 Sep 22 10:35 CMakeCache.txt
// mi hijo ha muerto
// hola que tal