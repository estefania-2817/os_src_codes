#include "functions.h"

#include <ctype.h>
#include <stdio.h>

// definición de la función
void ModifyArray(char *array) {
  for (int i = 0; array[i] != '\0'; i = i + 2) {
    array[i] = toupper(array[i]);
  }
}