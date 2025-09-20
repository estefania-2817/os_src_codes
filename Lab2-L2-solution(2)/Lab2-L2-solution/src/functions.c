#include "functions.h"

#include <ctype.h>
#include <string.h>

void ModifyArray(char *array) {
  for (int i = 0; i < strlen(array); i = i + 2) {
    array[i] = toupper(array[i]);
  }
}