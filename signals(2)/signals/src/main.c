// #define _POSIX_C_SOURCE 200809L  // I ADDDED THIS TO REMOVE SA WARNING
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void gestionador(int signum) {
  // takes signal numper as a parameter and prints it
  printf("Received signal %d\n", signum);
  // fflush(stdout);  // I ADDED This ensures the message appears
}

void gestionador_term(int signum) {
  // does the same but for SIGTERM
  printf(" I won't die!!!. Received signal %d\n", signum);
  // fflush(stdout);  // I ADDED This ensures the message appears
}

int main() {
  int i = 0;

  struct sigaction sa;
  sa.sa_handler = gestionador;
  sigaction(SIGINT, &sa, NULL);

  struct sigaction sa2;
  sa2.sa_handler = gestionador_term;
  sigaction(SIGTERM, &sa2, NULL);

  while (1) {
    printf("wasting %d cycles. %d\n", i++, getpid());
    sleep(8);
  }

  return 0;
}