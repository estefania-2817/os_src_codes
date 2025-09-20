#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  // Check if the number of arguments is correct.
  // argc should be 3: the program name itself + two numbers.
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <number1> <number2>\n", argv[0]);
    return 1;
  }

  // Convert the string arguments to floating-point numbers.
  float num1 = atof(argv[1]);
  float num2 = atof(argv[2]);

  // Add the numbers.
  float result = num1 + num2;

  // Print the result.
  printf("The sum of %s and %s is %.2f\n", argv[1], argv[2], result);

  return 0;
}