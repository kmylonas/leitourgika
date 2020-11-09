
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
   if(tid == 0 || args==NULL){
     return NOTHREAD;
   }
   else
  rlist_push_front(&CURPROC->ptcb_list ,&ptcb->ptcb_list_node);
  CURPROC->thread_count++;
  ptcb->tcb->owner_pcb = CURPROC;
  wakeup(ptcb->tcb);
  
  return tid;

}


/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
<<<<<<< HEAD
  Tid_t tid = (Tid_t)(CURTHREAD->ptcb);
	return tid;
=======
	return (Tid_t) cur_thread();
>>>>>>> 7c951b6e7a4655f1bddf226142737cec8fd88425
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

