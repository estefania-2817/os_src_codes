#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int balance;   // account balance
  char name[20];  // account name 
} Account;

Account account_a;   //global var, account instance
Account account_b;   

pthread_mutex_t mutex;  //global mutex  

void InitAccount(int balance, char* name, Account* account) {
  account->balance = balance; //set initial balance 
  strcpy(account->name, name); //copy acc name
}

void* tranfer1(void* arg) {
  pthread_mutex_lock(&mutex); // acquire lock
  account_b.balance += 150;   // add 150 to b
  account_a.balance -= 150;   // sub 150 to a
  pthread_mutex_unlock(&mutex); // release lock
}

void* tranfer2(void* arg) {
  pthread_mutex_lock(&mutex);  // acquire lock
  account_b.balance -= 300;    // sub 300 to b
  account_a.balance += 300;    // add 300 to a
  pthread_mutex_unlock(&mutex); // release lock
}

int main() {
  pthread_t thread1;   //thread handle
  pthread_t thread2;

  pthread_mutex_init(&mutex, NULL);   //intialize mutex with default attributes

  InitAccount(300, "Account A", &account_a);  //initialize accounts with balances
  InitAccount(500, "Account B", &account_b);

  int create = pthread_create(&thread1, NULL, tranfer1, NULL);  // create threads
  int create2 = pthread_create(&thread2, NULL, tranfer2, NULL);
      // pthread_create(type, pthread_attr_t*, 
      //                void* (*)(void*) (function pointer, function thread will execute), 
      //                void* (generic pointer, argument to pass to function));


  if (create == -1 || create2 == -1) {
    perror("error pthreade create: ");
  }

  pthread_join(thread1, NULL);  // wait for threads to complete
  pthread_join(thread2, NULL);

  printf("balance:  %d    name: %s \n", account_a.balance, account_a.name);
  printf("balance:  %d    name: %s \n", account_b.balance, account_b.name);
}