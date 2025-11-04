#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

// --- Configuration ---
#define BLOCK_SIZE_KB 64  // Size of each block in Kilobytes
#define BLOCK_SIZE (BLOCK_SIZE_KB * 1024) // convert kb to bytes
#define PROC_MAPS_PATH "/proc/self/maps" //patht to process memory mapp

// Structure to hold the details of each memory block
typedef struct {
  char name[20]; 
  int *address;
  size_t size;
  char os_map_range[30];  // Stores the range read from /proc/maps
} MemoryBlock;

// Global array to store block information
MemoryBlock blocks[4];

// --- Helper Functions ---

/**
 * @brief Reads /proc/self/maps and finds the mapped range for a given address.
 * @param addr The start address reported by mmap.
 * @param output_buffer Buffer to store the resulting range (e.g.,
 * 7fff0000-7fff4000).
 */
void GetOsMapRange(void *addr, char *output_buffer) { //get mem adrr and find where os mapped it
  FILE *f = fopen(PROC_MAPS_PATH, "r");
  char line[256];
  int64_t start, end;

  // Default value if not found
  strncpy(output_buffer, "NOT_FOUND", 30);

  if (!f) {
    strncpy(output_buffer, "FILE_ERROR", 30);
    return;
  }

  // Convert the mmap address to a string prefix for searching
  char addr_prefix[20];
  // Format the address as hexadecimal string
  snprintf(addr_prefix, sizeof(addr_prefix), "%lx", (int64_t)addr);

  // Read the maps file line by line
  while (fgets(line, sizeof(line), f)) {
    // Read the start and end addresses from the line
    if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
      // Check if the reported address falls within this kernel-mapped range
      if ((int64_t)addr >= start && (int64_t)addr < end) {
        // Found the corresponding line in /proc/self/maps
        // Format the output range
        snprintf(output_buffer, sizeof(blocks[0].os_map_range), "%lx-%lx",
                 start, end);
        fclose(f);
        return;
      }
    }
  }

  fclose(f);
}

// --- Core Task Function ---

/**
 * @brief Creates four distinct memory blocks using mmap().
 * * REQUIRED POSIX FUNCTION: mmap(void *addr, size_t length, int prot, int
 * flags, int fd, off_t offset)
 * * TASK: Create the following four memory blocks, storing the returned address
 * in the blocks array:
 * * 1. BLOCK 0 (Auto-Assigned): Use NULL for the addr parameter.
 * 2. BLOCK 1 (Another Auto-Assigned): Use NULL for the addr parameter again.
 * 3. BLOCK 2 (Hinted Address 1): Use a specific, high-address hint (e.g.,
 * 0x70000000) for the addr parameter, using the MAP_FIXED flag to make the
 * request mandatory.
 * 4. BLOCK 3 (Hinted Address 2): Use a different specific, high-address hint
 * (e.g., 0x80000000) for the addr parameter, using the MAP_FIXED flag.
 * * NOTE: MAP_FIXED is aggressive and may fail if the address is already in
 * use.
 */
void CreateMemoryBlocks() {
  // Step 1: Initialize the data structure
  strncpy(blocks[0].name, "BLOCK_0_AUTO_A", 20);
  blocks[0].size = 64 * 1024;  // 64 KB to bytes

  strncpy(blocks[1].name, "BLOCK_1_AUTO_B", 20);
  blocks[1].size = 128 * 1024;  // 128 KB to bytes

  strncpy(blocks[2].name, "BLOCK_2_HINT_1", 20);
  blocks[2].size = 64 * 1024;  // 64 KB to bytes

  strncpy(blocks[3].name, "BLOCK_3_HINT_2", 20);
  blocks[3].size = 64 * 1024;  // 64 KB to bytes

  // Step 2: Create memory blocks using mmap()
  // Block 0: Auto-assigned, 64 KB // null let OS choose any available address
  blocks[0].address = mmap(NULL, blocks[0].size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (blocks[0].address != MAP_FAILED) {
    blocks[0].address[0] = 100;  // Write data to first position
    GetOsMapRange(blocks[0].address, blocks[0].os_map_range);
  }

  // Block 1: Auto-assigned, 128 KB // null let OS choose any available addres
  blocks[1].address = mmap(NULL, blocks[1].size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (blocks[1].address != MAP_FAILED) {
    blocks[1].address[0] = 200;  // Write data to first position
    GetOsMapRange(blocks[1].address, blocks[1].os_map_range);
  }

  // Block 2: Fixed at 0x70000000, 64 KB
  blocks[2].address =
      mmap((void *)0x70000000, blocks[2].size, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  if (blocks[2].address != MAP_FAILED) {
    blocks[2].address[0] = 300;  // Write data
    GetOsMapRange(blocks[2].address, blocks[2].os_map_range);
  } else {
    // Handle MAP_FIXED failure - try without MAP_FIXED as fallback
    blocks[2].address =
        mmap((void *)0x70000000, blocks[2].size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (blocks[2].address != MAP_FAILED) {
      blocks[2].address[0] = 300;
      GetOsMapRange(blocks[2].address, blocks[2].os_map_range);
    }
  }

  // Block 3: Fixed at 0x80000000, 64 KB
  blocks[3].address =
      mmap((void *)0x80000000, blocks[3].size, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  if (blocks[3].address != MAP_FAILED) {
    blocks[3].address[0] = 400;  // Write data
    GetOsMapRange(blocks[3].address, blocks[3].os_map_range);
  } else {
    // Handle MAP_FIXED failure - try without MAP_FIXED as fallback
    blocks[3].address =
        mmap((void *)0x80000000, blocks[3].size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (blocks[3].address != MAP_FAILED) {
      blocks[3].address[0] = 400;
      GetOsMapRange(blocks[3].address, blocks[3].os_map_range);
    }
  }
}

/**
 * @brief Prints a single block entry for the final log line.
 * @param pid The PID of the current process (kept for compatibility if needed).
 * @param block Pointer to the MemoryBlock to print.
 */
void PrintResultLog(pid_t pid, MemoryBlock *block) {
  printf("%s=%p/%zu/%d/%s|", block->name, (void *)block->address, block->size,
         block->address[0], block->os_map_range);
}

int main() {
  pid_t pid = getpid();

  // Create the memory blocks
  CreateMemoryBlocks();

  // Print the required log line using a per-block function
  printf("LOG_START|PID=%d|", pid);
  for (int i = 0; i < 4; i++) {
    PrintResultLog(pid, &blocks[i]);
  }
  printf("LOG_END\n");

  // Wait for validation script to run. The script will kill the process.
  // We use a safe sleep here to prevent the process from exiting immediately.
  sleep(10);

  // Clean up
  for (int i = 0; i < 4; i++) {
    if (blocks[i].address != MAP_FAILED) {
      munmap(blocks[i].address, blocks[i].size);
    }
  }

  return 0;
}
