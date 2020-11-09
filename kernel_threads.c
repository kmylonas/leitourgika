
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "util.h"





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
	return -1;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
	return -1;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{

}

