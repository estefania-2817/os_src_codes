# os_src_codes
example exercises

My notes from my study sessions

#Include
* stdio.h - standard input/output (printf, fprintf, fgets, stdin, stdout, 
  stderr)
* stdlib.h - standard library, EXIT_SUCCESS, EXIT_FAILURE, memory allocation
* string.h - string handling
* ctype.h - character handling (isalpha[to know if its a letter], toupper)
* unistd.h - POSIX OS API (sleep(), file operations, fork, getpid)
* time.h - date and time
* signal.h - signal handling (signal(), sigaction(), or signal interrupt 
  like crtl+C)
* sys/types.h - data types used in calls, pid_t
* wait.h 
* fcntl.h - file control, O_CREATE, OWRONLY
* mqueue.h - POSIX message queue functions
* sys/stat.h - file status, permissions

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * i

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

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * i

array exercise
* char in_array[100] -- the size is 100 for a max inf limit
* fget() -  file get string - standard library function, read a string of 
  text from input stream (keyboard or file) and store in character array
* * char *fgets(char *str, int n, FILE *stream); (FILE *stream in this case the keyboard or user input, it could have been a file)
* * void ModifyArray(char *array) {} - pointer of the address memory of a 
    certain char in the given array

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * i

stdio_stderr
* printf() - standard output, automatically writes to stdout
* fprintf - general version, lets you chose the output stream
* fflush(stdout) - output buffered for efficiency, ensures to be printed in correct order
* * without flush, the stderr appears to be the first to print
* fprintf(stderr, "xxx") - standard error stream

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * i

fork_intro
* "pid_t pid;" - process id type (typedef for int)
* pid = getpid();  // get current process's PID
* pidX = fork();  // created child process
* After fork(), two identical processes exist
* The return value of fork() determines whether you're in parent or child
* Both processes continue execution independently
* They share the same code but have separate memory spaces
* * memory visualization
* * BEFORE fork()
* * * parent process
* * * * pid = 1234
* * * * pid2 = uninitialized
* * AFTER fork()
* * * parent process
* * * * pid = 1234
* * * * pid2 = 1235 (child)
* * * child process
* * * * pid = 1235
* * * * pid2 = 0, indicating child

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * i

fork_intro_v2
* execlp() replaces the current process's memory space with a new program, 
  if successful, the child process becomes "htop" completely
* wait(NULL) makes the parent pause until the child terminates 
* * This prevents "zombie processes" (completed processes whose 
    parents haven't waited for them)
* execlp("htop", "htop_cooler", NULL)
* * "htop": The actual program to execute (searches PATH for 'htop')
* * "htop_cooler": What the program will see as its name (argv[0])
* * NULL: End of argument list

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * i

message queues
* mq_open() - Creates/opens a message queue
* mq_send() - Sends a message to the queue
* mq_receive() - Receives a message from the queue
* mq_unlink() - Removes a message queue (commented 
  out)
* O_CREAT - Create the queue if it doesn't exist
* O_WRONLY/O_RDONLY - Write-only/Read-only access
* 0644 - File permissions (read/write for owner, 
  read for others)
* mq_maxmsg - Maximum number of messages in 
  queue (5)
* mq_msgsize - Maximum size of each message (8192 
  bytes)
* mq_curmsgs - Current number of messages in queue
* &attr - is my variable for my struct mq_attr
* * *mq_send(queue_os,        // Queue descriptor
                message,      // Message buffer
                8192,         // Message size (using full buffer size)
                0);           // Priority (0 = normal)
* DIFFERENCE between snprintf and printf
* * printf - sends formatted output directly to the standard output 
    (usually the terminal)
* * snprintf - formats a string into a buffer with size protection
    in this case "char message[8192]" us the buffer for the message