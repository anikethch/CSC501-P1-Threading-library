#include <stdio.h>
#include <ucontext.h>
#include <malloc.h>
#include <signal.h>
#include <stdlib.h>
#include "mythreadlib.h"
#include "mythread.h"
#define thread_stack_size 8*1024

int tid;
MyThread_ *t_list[QUEUE_MAX*3];

MyThread_ *parent,*running;
ucontext_t *mp;

//dequeue thread id from queue and map it to corresponding thread from t_list
MyThread_ *head_of_queue (int queue_type) {
    int t;
    MyThread_ *temp;
    t=dequeue(0);
    if (t == 0) {
        return NULL;  
    } else {
        temp = t_list[t--];
        if (temp->id < 1 || temp->id > QUEUE_MAX+1) 
        return NULL;
        return temp;
    }
}

// creates new thread
MyThread MyThreadCreate (void (*start_funct)(void *),void *args) {
    MyThread_ *new_t,*temp;
    if (tid > (THREAD_MAX+1)) {
        printf ("Maxmimum number of threads exceeded\n");
        return ;
    }
    new_t =  malloc(sizeof(MyThread_));
    new_t -> cxt = malloc (sizeof(ucontext_t));
    new_t -> id = tid;
    t_list[tid] = new_t;
    tid++ ;
    new_t->block_status=0;
    new_t->child=NULL;
    new_t->sib=NULL;
    new_t->parent_t = running;
    if (running->child == NULL) {
        running->child = new_t;
    } else {
        temp = running -> child;
        while (temp -> sib != NULL) {
           temp = temp -> sib;
        }
        temp->sib = new_t;   
    }
    new_t->cxt->uc_link = NULL ;
    new_t->cxt->uc_stack.ss_sp = malloc(thread_stack_size);
    new_t->cxt->uc_stack.ss_flags = 0;
    new_t->cxt->uc_stack.ss_size = thread_stack_size;
    getcontext(new_t->cxt);
    makecontext(new_t->cxt,(void (*)())start_funct,1,args);
    enqueue(0,new_t->id);
    MyThread *return_t = (MyThread*)new_t;
    return *return_t;
}

void MyThreadYield(void) {
    MyThread_ *t,*temp;
    int parent_match=0;
    if (running->id ==1)
    parent_match=1;
    t = running;
    getcontext(t->cxt);
    temp = running;
    temp = head_of_queue(0);
    if (temp != NULL) {
    if (t->id!=1) {
    enqueue(0,t->id);
    }
    running = temp;
    if (!parent_match)
    swapcontext(t->cxt,running->cxt);
    else 
    swapcontext(parent->cxt,running->cxt);
    } else if ((parent!=NULL) && (parent->block_status==0)) {
    running=parent;
    setcontext(running->cxt);
    } else {
    setcontext(mp);
    }
}

void MyThreadInit (void(*start_funct)(void *), void *args) {
    MyThread_ *temp;
    tid = 1;
    parent = (MyThread_ *) malloc(sizeof(MyThread_)) ;
    if (parent == NULL) {
    exit(-1);
    }
    parent->cxt =  (ucontext_t *)malloc(sizeof(ucontext_t));
    if (parent->cxt == NULL) {
    exit(-1);
    }
    parent->id = tid ;
    t_list[tid] = parent;
    tid++;
    parent->block_status = 0;
    parent->child = NULL;
    parent->sib = NULL;
    parent->parent_t = NULL;
    parent->cxt->uc_link = NULL ;
    parent->cxt->uc_stack.ss_sp = malloc (thread_stack_size);
    parent->cxt->uc_stack.ss_flags = 0 ;
    parent->cxt->uc_stack.ss_size = thread_stack_size; 
    getcontext(parent->cxt);
    makecontext(parent->cxt,(void (*)())start_funct,1,args);
    mp = (ucontext_t *)malloc (sizeof(ucontext_t));
    mp -> uc_link = NULL ;
    mp -> uc_stack.ss_sp = malloc(thread_stack_size);
    mp -> uc_stack.ss_flags = 0;
    mp -> uc_stack.ss_size =  thread_stack_size;
    running = parent;
    swapcontext(mp,parent->cxt);
}

void MyThreadExit(void) {

     MyThread_ *temp,*t,*t2;
     temp =  running;
     t2 = running;

     //check if the current thread is blocking the parent and unblock it if this is the only thread blocking parent     
     if (running->block_status ==0) { 
     if(running->parent_t != NULL) {
     if((running->parent_t->block_status>0 && search_blocking_child(running->id))) {
          running->parent_t->block_status--;
     if(running->parent_t->block_status ==0) {
          if (running->parent_t->id !=0 ) 
          enqueue(0,running->parent_t->id);
     }
     }
     }

     //remove parent child relationship
     if (running->sib != NULL) {

     if (running->parent_t!=NULL) {
         t2 = (running->parent_t)->child;
         if (t2==running) {
             t2->parent_t->child = t2->sib;
         } else {
         while ((t2->sib)->id!=running->id) { 
             t2 = t2->sib; 
         }
         t2->sib = running->sib;
     }
     }
     }
  
     if (running->id==1) {
     free (running->cxt->uc_stack.ss_sp);
     free (running->cxt);
     free (running);
     setcontext(mp);

     } else {

     free (running->cxt->uc_stack.ss_sp);
     free (running->cxt);
     free (running);


     t = head_of_queue(0);
     if (t != NULL) {
         running = t;
         setcontext(running->cxt);
     } else if ((parent!=NULL) && (parent->block_status==0)) {
         running=parent;
         setcontext(parent->cxt);
     } else {
         setcontext(mp);
     }
   }
  }
}

int MyThreadJoin(MyThread thread) {

    MyThread_ *t_ptr = (MyThread*)&thread;
    MyThread_ *t,*temp;
    temp = running;
    t = running;
    int find = 0;

    if(running->id == t_ptr->parent_t->id)  
    find=1 ;
    
    if (find==0)  
         return(-1);
    else {
         getcontext(t->cxt);
         running->block_status++;
         enqueue(1,t_ptr->id); 
         temp = head_of_queue(0);
         if (temp==NULL) {
             setcontext(mp); 
         } else {
             running = temp;
             swapcontext(t->cxt,running->cxt);
         } 
    }
}

void MyThreadJoinAll(void) {

     MyThread_ *t,*temp,*t2;
     temp = running;
     t = running -> child;
     if (t!=NULL) {
        while(t!=NULL) {
             if(search_blocking_child(t->id)==0) {
                   running->block_status++;
                   enqueue(1,t->id);
             }   
             t=t->sib;
        }
        t2=running;
        getcontext(t2->cxt);
        getcontext(temp->cxt);
        temp = head_of_queue(0);
	if (temp == NULL) {
           setcontext(mp);
        } else {
           running=temp;
           swapcontext(t2->cxt,running->cxt);
        }
     } 
}

MySemaphore MySemaphoreInit(int initialValue) {

     MySemaphore_ *sem;
     sem = (MySemaphore_*) malloc(sizeof(MySemaphore_));
     
     if (sem==NULL){
         printf("Semaphore creation failed\n");
     } else {
        if (initialValue < 0) {
             printf("initialValue of semaphore should be non-negative value\n");
             return;
        } else {
             sem->val = initialValue;
             sem->destroyed = 0;
             sem->front = -1;
             sem->rear = -1;
             MySemaphore *return_sem = (MySemaphore*)sem;
             return *return_sem;
        }
     }
}

void MySemaphoreSignal(MySemaphore sem) {
     MySemaphore_ *tempsem = (MySemaphore*)&sem;
     MyThread_ *temp;
     int tid;
     temp=running;
     if (!tempsem->destroyed) {
     tempsem->val++;
     if (tempsem->val<=0) {
         tid=semdequeue(tempsem);
         if (tid!=0) {
             temp= t_list[tid--];
             enqueue(0,temp->id);
         }
     }
    }
}

void MySemaphoreWait(MySemaphore sem) {
     MySemaphore_ *tempsem = (MySemaphore*)&sem;
     MyThread_ *temp,*t;
     t=running;
     getcontext(t->cxt);
     if (!tempsem->destroyed) {
     tempsem->val--;
     if (tempsem->val<0) {
         semenqueue(tempsem,t->id);
         temp=running;
         temp=head_of_queue(0);
         if (temp != NULL) {
             running = temp;
             swapcontext(t->cxt,running->cxt);
         } else {
             setcontext(mp);
         }
     }
     }
}

int MySemaphoreDestroy(MySemaphore sem) {

     MySemaphore_ *tempsem = (MySemaphore*)&sem;
     if (tempsem->front==-1 || tempsem->front > tempsem->rear) {
        tempsem->destroyed=1;
        return 0 ;
     } else {
        printf("Threads are block_status on this semaphore\n");
        return -1;
     }
}
