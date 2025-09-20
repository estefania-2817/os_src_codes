#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void gestionador(int signum) { printf("Received signal %d\n", signum); }
void gestionador_term(int signum) {
  printf(" I won't die!!!. Received signal %d\n", signum);
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
    sleep(1);
  }

  return 0;
}
