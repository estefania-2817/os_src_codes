#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Shared resources: Two bank accounts
typedef struct {
  int balance;
  pthread_mutex_t mutex;
  char name[10];
} Account;

// Global accounts
Account account_A;
Account account_B;

// Function to transfer money from one account to another
void transfer(Account* from, Account* to, int amount) {
  // Determine the lock order to avoid deadlock
  pthread_mutex_t* first_mutex = &from->mutex;
  pthread_mutex_t* second_mutex = &to->mutex;

  // if (strcmp(from->name, to->name) > 0) {
  if (&from->mutex > &to->mutex) {
    first_mutex = &to->mutex;
    second_mutex = &from->mutex;
  } else {
    first_mutex = &from->mutex;
    second_mutex = &to->mutex;
  }

  ////Acquire locks in a consistent order pthread_mutex_lock(first_mutex);
  printf("Thread %ld: Locked %s. Trying to lock %s.\n", pthread_self(),
         from->name, to->name);
  sleep(1);  // Simulate a delay
  pthread_mutex_lock(second_mutex);
  printf("Thread %ld: Locked %s and %s. Transferring %d.\n", pthread_self(),
         from->name, to->name, amount);

  // Critical section
  if (from->balance >= amount) {
    for (int i = 0; i < amount; i++) {
      from->balance -= 1;
      to->balance += 1;
      usleep(10000);  // Simulate processing time
    }

    printf("Thread %ld: Transfer successful.\n", pthread_self());
  } else {
    printf("Thread %ld: Insufficient funds for transfer.\n", pthread_self());
  }

  // Unlock in reverse order
  pthread_mutex_unlock(second_mutex);
  pthread_mutex_unlock(first_mutex);
  return;
}

// Transaction 1: Transfer from A to B
void* transaction1_func(void* arg) {
  transfer(&account_A, &account_B, 50);
  return NULL;
}

// Transaction 2: Transfer from B to A
void* transaction2_func(void* arg) {
  transfer(&account_B, &account_A, 30);
  return NULL;
}

int main() {
  pthread_t thread1, thread2;
  pthread_t thread3, thread4;

  // Initialize accounts and mutexes
  account_A.balance = 100;
  account_B.balance = 200;
  strcpy(account_A.name, "Account A");
  strcpy(account_B.name, "Account B");
  pthread_mutex_init(&account_A.mutex, NULL);
  pthread_mutex_init(&account_B.mutex, NULL);

  printf("Initial Balances: Account A = %d, Account B = %d\n",
         account_A.balance, account_B.balance);

  // Create threads
  pthread_create(&thread1, NULL, transaction1_func, NULL);
  pthread_create(&thread2, NULL, transaction1_func, NULL);
  pthread_create(&thread3, NULL, transaction2_func, NULL);
  pthread_create(&thread4, NULL, transaction2_func, NULL);

  // Wait for threads to finish
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);
  pthread_join(thread4, NULL);

  printf("Final Balances: Account A = %d, Account B = %d\n", account_A.balance,
         account_B.balance);

  // Destroy mutexes
  pthread_mutex_destroy(&account_A.mutex);
  pthread_mutex_destroy(&account_B.mutex);

  return 0;
}