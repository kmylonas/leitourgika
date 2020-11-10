
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "util.h"
#include "kernel_cc.h"





/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{
  
  PTCB* ptcb=initialize_ptcb();
  ptcb->task = task;
  ptcb->argl = argl;
  ptcb->args = args;
  
  ptcb->tcb  = spawn_thread(CURPROC,ptcb,start_thread);
  Tid_t tid=(Tid_t)(ptcb->tcb->ptcb);

  rlist_push_front(&CURPROC->ptcb_list ,&ptcb->ptcb_list_node);
  CURPROC->thread_count++;
  
  wakeup(ptcb->tcb);
  
  return tid;

}


/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
  Tid_t tid = (Tid_t)(cur_thread()->ptcb);
	return tid;
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{
  PCB* curproc = CURPROC;
  PTCB *ptcb = (PTCB*)tid;
  rlnode* node = rlist_find(&curproc->ptcb_list, ptcb, NULL);//ptcb_list_node
  //Tid_t node_tid = (Tid_t)(node->obj);
  if(node == NULL){
    printf("No thread with tid %u found\n", tid);
    return -1;
  }
  while(node->ptcb->exited==0 && node->ptcb->detached==0){//T2 not exited and T2 no detached
    kernel_wait(node->ptcb->exit_cv, SCHED_USER);//T1 sleeps, it will return here when we broadcast it(in thread exit)
  }
  exitval=node->ptcb->exitval;
  



	return -1;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  PCB* curproc = CURPROC;
  PTCB *ptcb = (PTCB*)tid;
  rlnode* node = rlist_find(&curproc->ptcb_list, ptcb, NULL);//node=ptcb_list_node
  Tid_t node_tid = (Tid_t)node->obj;
  if(node->ptcb->detached==1 || node==NULL || node->ptcb->exited==1)
	  return -1;
  else
  {
    node->ptcb->detached = 1;
    return 0;
  }
  
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{
  T2->exited=1
broadcast!!
}

