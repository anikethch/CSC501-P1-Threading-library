#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#define THREAD_MAX 10
#define QUEUE_MAX 40

extern int tid;

typedef struct Thread_ { 

   int id ;
   ucontext_t *cxt; 
   int block_status; //number of children that blocked this thread
   struct Thread_ *child; // maintain list of children
   struct Thread_ *sib; //maintain list of children
   struct Thread_ *parent_t; //pointer to parent
}MyThread_;

typedef struct MySem_ {
    int val; //to maintain value of semaphore
    int destroyed; //  to maintain destroy status of semaphore
    int sem_block_queue[QUEUE_MAX]; //block queue of semaphore
    int front; //head index of queue
    int rear; //rear index of queue
}MySemaphore_;

void enqueue(int queue_type, int tid);

int dequeue(int queue_type);

int search_blocking_child(int id);

void semenqueue(MySemaphore_ *sem,int tid);

int semdequeue(MySemaphore_ *sem);
