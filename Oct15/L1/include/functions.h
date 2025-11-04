#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

// --- Configuration ---
// 500x500 is typically enough to take over 1 second on modern CPUs.
#define MATRIX_SIZE 500

void* HeavyCpuTask(void* arg);
int SetNiceValue(int nice_val);
int SetCpuAffinity(int cpu_num);

#endif  // INCLUDE_FUNCTIONS_H_