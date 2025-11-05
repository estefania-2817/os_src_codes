/*
 exam_2 - memory "fight" between two threads and a monitor thread.
 Compile: cmake + make (CMakeLists provided)
 Author: delivered as assignment solution example
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sched.h>

static int *memblock = NULL; //pointer to shared memory block
static size_t NE = 0; //number of elements in memblock
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // mutex for protecting memblock and counters
static size_t done_count = 0; /* number of elements which are -1 or 99 */
static volatile int finished = 0; //global flag used to signal all threads to stop

/* Helpers to set thread affinity to a single CPU */
int set_thread_cpu(unsigned cpu) { //binds current thread to specified CPU core
    cpu_set_t cpuset;                    //prevents os from scheduling thread on other cores
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

/* Helper to set thread nice value. Uses thread TID (linux) */
int set_thread_nice(int niceval) { //adjust nice value of current thread
    pid_t tid = (pid_t) syscall(SYS_gettid); //sysgettid gets thread id in linux
    /* setpriority returns 0 on success, -1 on failure (and errno set) */
    if (setpriority(PRIO_PROCESS, (int)tid, niceval) == -1) {
        return -1;
    }
    return 0;
}

/* Worker that writes target_val (-1 or 99) and honors mutex and termination conditions.
   It uses a thread-local seed for rand_r so we call srand() once in main and derive seeds there. */
struct worker_args {
    int target_val;  // value to write (-1 or 99)
    unsigned int seed; // random seed for rand_r
    int niceval;  //priority nice value for this thread
    unsigned cpu; //cpu core to bind this thread to
};

void *worker_thread(void *arg) {
    //struct worker_args *a = (struct_worker_args_type_stub *)arg; /* replaced below (we'll cast properly) */
    /* The above stub will be fixed by correct cast - C doesn't like unknown type names in forward declaration.
       We'll instead cast directly to known type: */
    (void)0; // ????????????
    struct worker_args *w = (struct worker_args *)arg; // Casts the input pointer to a worker_args struct.

    /* set affinity */
    if (set_thread_cpu(w->cpu) != 0) {
        fprintf(stderr, "Warning: failed to set thread CPU affinity (errno=%d): %s\n", errno, strerror(errno));
        /* continue even if fails */
    }

    /* set per-thread nice */
    if (set_thread_nice(w->niceval) != 0) {
        fprintf(stderr, "Warning: failed to set nice=%d for thread (errno=%d): %s\n", w->niceval, errno, strerror(errno));
    }

    unsigned int seed = w->seed; // each thread has its own seed
    int target = w->target_val;

    while (!finished) {
        /* pick random index using thread-local rand_r */
        size_t idx = (size_t)(rand_r(&seed) % NE);

        pthread_mutex_lock(&lock);
        if (!finished) {
            int cur = memblock[idx];
            if (cur != -1 && cur != 99) { //skip cells already written by either thread
                memblock[idx] = target; //write target value at chosen index ???
                done_count++;
                /* if we've finished all elements, set finished flag */
                if (done_count >= NE) {
                    finished = 1;
                }
            }
        }
        pthread_mutex_unlock(&lock);

        /* optional small pause to allow thread scheduling differences to be visible */
        /* usleep(1); */ /* commented out to maximize work (uncomment for debugging) */
    }

    return NULL; // Thread exits cleanly when finished == 1.
}

/* Monitor thread: CPU 0, copies memory block under mutex and prints percentages.
   Ends when finished is set. */
void *monitor_thread(void *arg) {
    (void)arg;
    if (set_thread_cpu(0) != 0) {
        fprintf(stderr, "Warning: failed to set monitor CPU affinity (errno=%d): %s\n", errno, strerror(errno));
    } // Binds the monitor thread to CPU 0.

    int *local_copy = malloc(sizeof(int) * NE);
    if (!local_copy) {  // Local buffer to read a snapshot of shared 
        perror("malloc");       // memory without interfering with workers.
        return NULL;
    }

    while (!finished) {  
        pthread_mutex_lock(&lock); // Copies the entire shared block 
        for (size_t i = 0; i < NE; ++i) {   // under protection of the mutex.
            local_copy[i] = memblock[i];
        }
        pthread_mutex_unlock(&lock);

        /* count -1 and 99 */
        size_t cnt_neg1 = 0, cnt_99 = 0;   // Calculates percentage of memory claimed by each thread.
        for (size_t i = 0; i < NE; ++i) {
            if (local_copy[i] == -1) cnt_neg1++;
            else if (local_copy[i] == 99) cnt_99++;
        }
        double pct_neg1 = (NE > 0) ? (100.0 * cnt_neg1 / (double)NE) : 0.0;
        double pct_99  = (NE > 0) ? (100.0 * cnt_99  / (double)NE) : 0.0;
        printf("[Monitor] -1: %.2f%% (%zu)   99: %.2f%% (%zu)   done_count=%zu\n",
               pct_neg1, cnt_neg1, pct_99, cnt_99, done_count);

        /* stop if finished (checked at top of loop), otherwise sleep a bit to avoid busy cycle */
        if (!finished) {
            struct timespec ts = {0, 100 * 1000 * 1000}; /* 100 ms */
            nanosleep(&ts, NULL);
        }
    }

    /* one final copy/print to show 100% */ // Recounts one last time to ensure final totals are printed.
    pthread_mutex_lock(&lock);
    for (size_t i = 0; i < NE; ++i) local_copy[i] = memblock[i];
    pthread_mutex_unlock(&lock);

    size_t cnt_neg1 = 0, cnt_99 = 0;
    for (size_t i = 0; i < NE; ++i) {
        if (local_copy[i] == -1) cnt_neg1++;
        else if (local_copy[i] == 99) cnt_99++;
    }
    double pct_neg1 = (NE > 0) ? (100.0 * cnt_neg1 / (double)NE) : 0.0;
    double pct_99  = (NE > 0) ? (100.0 * cnt_99  / (double)NE) : 0.0;
    printf("[Monitor FINAL] -1: %.2f%% (%zu)   99: %.2f%% (%zu)   done_count=%zu\n",
           pct_neg1, cnt_neg1, pct_99, cnt_99, done_count);

    free(local_copy);
    return NULL;
}

/* Fix: provide proper struct definition forward (we used it above) - it's already defined earlier. */

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s NE NICE_1 NICE_2\n", argv[0]);
        return 1;
    }

    NE = (size_t) strtoull(argv[1], NULL, 10);  //number of elements in shared memory block
    int nice1 = atoi(argv[2]); //nice values for the two worker threads
    int nice2 = atoi(argv[3]);

    if (NE == 0) {
        fprintf(stderr, "NE must be > 0\n");
        return 1;
    }

    /* mmap a memory block of NE integers */
    size_t byte_size = NE * sizeof(int);
    memblock = mmap(NULL, byte_size, PROT_READ | PROT_WRITE,  // Allocates a shared, anonymous memory region (not backed by a file).
                    MAP_ANONYMOUS | MAP_SHARED, -1, 0);   // Accessible by all threads
    if (memblock == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    for (size_t i = 0; i < NE; ++i) memblock[i] = 0; // initialize memory block to 0, all values are zero

    /* initialize mutex (already statically initialized) */

    /* srand once as hint suggests; use rand() to produce seeds for threads */
    srand((unsigned) time(NULL));
    unsigned s1 = (unsigned) rand();
    unsigned s2 = (unsigned) rand();
    unsigned s_monitor = (unsigned) rand();

    pthread_t t1, t2, tmonitor;
    struct worker_args wa1 = { .target_val = -1, .seed = s1, .niceval = nice1, .cpu = 1 };
    struct worker_args wa2 = { .target_val =  99, .seed = s2, .niceval = nice2, .cpu = 1 };

    /* start time measurement */
    struct timespec tstart, tend;
    clock_gettime(CLOCK_MONOTONIC, &tstart);

    /* create monitor thread (CPU 0) */
    if (pthread_create(&tmonitor, NULL, monitor_thread, NULL) != 0) {
        perror("pthread_create monitor");
        munmap(memblock, byte_size);
        return 1;
    }

    /* create worker threads */      //waz1 and wa2 contain the struct or 4 parameters for the two worker threads for the function worker_thread.
    if (pthread_create(&t1, NULL, worker_thread, &wa1) != 0) {
        perror("pthread_create t1");
        finished = 1;
    }
    if (pthread_create(&t2, NULL, worker_thread, &wa2) != 0) {
        perror("pthread_create t2");
        finished = 1;
    }

    /* wait for workers to finish */
    if (t1) pthread_join(t1, NULL);
    if (t2) pthread_join(t2, NULL);
    /* monitor should exit after finished becomes 1; join it */
    pthread_join(tmonitor, NULL);

    /* stop time */
    clock_gettime(CLOCK_MONOTONIC, &tend);
    double elapsed = (tend.tv_sec - tstart.tv_sec) + (tend.tv_nsec - tstart.tv_nsec) / 1e9;

    printf("\nAll threads finished. Elapsed time: %.6f seconds\n", elapsed);

    /* print first 50 elements (or all if less) */
    size_t upto = (NE < 50) ? NE : 50;
    printf("First %zu elements:\n", upto);
    for (size_t i = 0; i < upto; ++i) {
        printf("%d ", memblock[i]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
    if (upto % 10 != 0) printf("\n");

    /* cleanup */
    munmap(memblock, byte_size);
    pthread_mutex_destroy(&lock);

    return 0;
}
