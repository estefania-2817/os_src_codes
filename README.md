# os_src_codes
example exercises

My notes from my study sessions

#Include
* stdio.h - standard input/output (printf, fprintf, fgets, stdin, stdout, stderr)
* stdlib.h - standard library, EXIT_SUCCESS, EXIT_FAILURE, memory allocation
* string.h - string handling
* ctype.h - character handling (isalpha[to know if its a letter], toupper)
* unistd.h - POSIX OS API (sleep(), file operations)
* time.h - date and time
* signal.h - signal handling (signal(), sigaction(), or signal interrupt like crtl+C)

arguments example
* int main (int argc, char* argv[])
    argc is the number of arguments
    argv is the strings in each argument
* if number of arguments isnt 3 then print a usage explanation
    <number1> <number2>
* "atof(argv[x])" turns an argument into a float
* format specifier
* * %s - string
* * %.2f -  floating point with 2 decimal places
* * %d or %i - integer
* * %u - unsigned int
* * %c - char

array exercise
* char in_array[100] -- the size is 100 for a max inf limit
* fget() -  file get string - standard library function, read a string of text from input stream (keyboard or file) and store in character array
* * char *fgets(char *str, int n, FILE *stream); (FILE *stream in this case the keyboard or user input, it could have been a file)
* * void ModifyArray(char *array) {} - pointer of the address memory of a certain char in the given array

stdio_stderr
* printf() - standard output, automatically writes to stdout
* fprintf - general version, lets you chose the output stream
* fflush(stdout) - output buffered for efficiency, ensures to be printed in correct order
* * without flush, the stderr appears to be the first to print
* fprintf(stderr, "xxx") - standard error stream
