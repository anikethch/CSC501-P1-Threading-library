#include <stdio.h>
#include "mythreadlib.h"

int ready_queue[QUEUE_MAX];
int block_queue[QUEUE_MAX];
int ready_front = -1;
int ready_rear = -1;
int block_front = -1;
int block_rear = -1;

// Queue_type is 0 if it is ready_queue else 1 if it is block_queue
void enqueue(int queue_type, int tid) {
    
    if (queue_type==0) {
        if (ready_rear == QUEUE_MAX-1) {
             printf ("Maximum number of threads exceeded\n");
        } else {
            if (ready_front  == -1 ) 
            ready_front  = 0 ;
            ready_rear = ready_rear+1;
            ready_queue[ready_rear] = tid;
        }
    } else if (queue_type==1) {          
        if (block_rear == QUEUE_MAX-1) {
            printf ("Maximum number of threads exceeded");
        } else {
            if (block_front  == -1 )
            block_front  = 0 ;
            block_rear = block_rear+1;
            block_queue[block_rear] = tid;
        }
    }
}

// Queue_type is 0 if it is ready_queue else 1 if it is block_queue
int dequeue(int queue_type) {
    int t;
    if (queue_type == 0) {
       if (ready_front == -1 || ready_front > ready_rear) {
           t=0;
           return 0;
       } else {
           t =  ready_queue[ready_front];
           ready_front = ready_front +1 ;
           return t ;
       }
    } else if (queue_type ==1) {
       if (block_front == -1 || block_front > block_rear) {
           t=0;
           return 0;
       } else {
       t =  block_queue[block_front];
       block_front = block_front +1 ;
       return t ;
       }
    } 
}


int search_blocking_child(int id){
   int i;
   int count =0 ;
   for (i=0;i<QUEUE_MAX;i++) {
       if (block_queue[i] == id) {
       count++;
       }
   }
   if (count ==0)
   return 0;
   else
   return 1;
}

void semenqueue(MySemaphore_ *sem,int tid) {
     if (sem->rear==QUEUE_MAX-1) 
         printf("Semaphore block queue overflow\n");
     else {
            if (sem->front  == -1 )
            sem->front  = 0 ;
            sem->rear = sem->rear++;
            sem->sem_block_queue[sem->rear] = tid;
     }
}

int semdequeue(MySemaphore_ *sem) {
    int t;
    if (sem->front==-1 || sem->front > sem->rear) {
        printf("No thread in sem blocked queue\n");
        t=0;
    } else {
        t=sem->sem_block_queue[sem->front];
        sem->sem_block_queue[sem->front]=0;
        sem->front = sem->front ++ ;
    }
    return t;
}
