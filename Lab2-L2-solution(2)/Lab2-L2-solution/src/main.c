#define _POSIX_C_SOURCE 200809L  // I ADDDED THIS TO REMOVE SA WARNING
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// These functions execute when their respective signals are received
void ActorHandler(int signum) {
  printf("Actor (PID: %d): Received SIGUSR1 from the Director\n", getpid());
}
void DirectorHandler(int signum) {
  printf("Director (PID: %d): Received SIGUSR2 from the Actor\n", getpid());
}

int main() {
  pid_t pid;  // Process ID variable for storing fork() result

  printf("Director (PID: %d): The play is about to begin\n", getpid());

  pid = fork();

  if (pid == 0) {
    // actor code
    struct sigaction sa;
    sa.sa_handler = ActorHandler;   // ActorHandler function from above
    sigaction(SIGUSR1, &sa, NULL);  // Install handler for SIGUSR1 signal

    printf("Actor (PID: %d): I'm ready\n", getpid());

    pause();  // BLOCK here until a signal is received

    sleep(2);  // Wait 2 seconds (dramatic pause!)
    printf(
        "Actor (PID: %d): To be, or not to be, that is the question: Whether "
        "'tis nobler in the mind to suffer the slings and arrows of outrageous "
        "fortune, or to take arms against a sea of troubles, and by opposing "
        "end "
        "them.\n",
        getpid());

    printf("Actor (PID: %d): I'm finished, sending SIGUSR2 to the director\n",
           getpid());

    kill(getppid(), SIGUSR2);  // NOT TYPO, get parent process id (ppid)
  } else {
    // director code
    struct sigaction sa;
    sa.sa_handler = DirectorHandler;  // DirectorHandler function from above
    sigaction(SIGUSR2, &sa, NULL);

    sleep(1);
    printf("Director (PID: %d): I'll send SIGUSR1 to the actor to start\n",
           getpid());
    kill(pid, SIGUSR1);

    pause();

    sleep(2);
    printf("Director (PID: %d): Okay, We're ending the play\n", getpid());
  }
  return 0;
}
