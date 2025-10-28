#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int balance;
  char name[20];
} Account;

Account account_a;
Account account_b;

pthread_mutex_t mutex;

void InitAccount(int balance, char* name, Account* account) {
  account->balance = balance;
  strcpy(account->name, name);
}

void* tranfer1(void* arg) {
  pthread_mutex_lock(&mutex);
  account_b.balance += 150;
  account_a.balance -= 150;
  pthread_mutex_unlock(&mutex);
}

void* tranfer2(void* arg) {
  pthread_mutex_lock(&mutex);
  account_b.balance -= 300;
  account_a.balance += 300;
  pthread_mutex_unlock(&mutex);
}

int main() {
  pthread_t thread1;
  pthread_t thread2;

  pthread_mutex_init(&mutex, NULL);

  InitAccount(300, "Account A", &account_a);
  InitAccount(500, "Account B", &account_b);

  int create = pthread_create(&thread1, NULL, tranfer1, NULL);
  int create2 = pthread_create(&thread2, NULL, tranfer2, NULL);
  if (create == -1 || create2 == -1) {
    perror("error pthreade create: ");
  }

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("balance:  %d    name: %s \n", account_a.balance, account_a.name);
  printf("balance:  %d    name: %s \n", account_b.balance, account_b.name);
}