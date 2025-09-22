#include <stdio.h>
#include <stdlib.h>

void PrintMessages();

int main()
{
  PrintMessages();
  return 0;
}

void PrintMessages()
{
  // Print a message to stdout
  printf("This  is the first message printed to stdout (standard output).");

  //
  fprintf(stdout,
          "This  is the second message printed to stdout (standard output).");

  fflush(stdout);

  // Print a message to stderr
  fprintf(stderr, "This message is printed to stderr (standard error). ");

  printf("\n");
}
///
///
///
///  ./stdio_test > output.txt