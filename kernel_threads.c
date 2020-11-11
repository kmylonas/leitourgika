
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
  PCB* curproc = CURPROC;
  PTCB* ptcb=initialize_ptcb();
  ptcb->task = task;
  ptcb->argl = argl;
  ptcb->args = args;
  
  ptcb->tcb  = spawn_thread(curproc,ptcb,start_thread);
  Tid_t tid=(Tid_t)(ptcb->tcb->ptcb);

  rlist_push_front(&curproc->ptcb_list ,&ptcb->ptcb_list_node);
  curproc->thread_count++;
  
  wakeup(ptcb->tcb);
  
  return tid;

}


/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{ TCB* tcb = cur_thread();
  Tid_t tid = (Tid_t)tcb->ptcb;
	return tid;
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{ 
  if(tid==sys_ThreadSelf())
    return -1; 
  if(tid == NOTHREAD)
    return -1;
  
  PCB* curproc = CURPROC;
  PTCB *ptcb = (PTCB*)tid;
  rlnode* node = rlist_find(&curproc->ptcb_list, ptcb, NULL);//ptcb_list_node
  if(node == NULL )
    return -1;
  if(node->ptcb->exited==1){  //if its exited we take exitval and return. We mustnt let it go for kernel_sleep because its exited and nobody will wake it up
    exitval = &node->ptcb->exitval;
    rlist_remove(&node->ptcb->ptcb_list_node);
    free(node->ptcb);
    return 0;
  }
  //Tid_t node_tid = (Tid_t)(node->obj);
  if(node->ptcb->detached==1 || node->ptcb->exited==1 || tid == sys_ThreadSelf()){//
    printf("HeheError joining the thread with ID %u\n", tid);
    return -1;
  }
  node->ptcb->refcount++;
  while(node->ptcb->exited==0 && node->ptcb->detached==0){//T2 not exited and T2 no detached
    kernel_wait(&node->ptcb->exit_cv, SCHED_USER);//T1 sleeps, it will return here when we broadcast it(in thread exit)
  }
    node->ptcb->refcount--;
    exitval=&node->ptcb->exitval; 
	return 0;
  
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  if(tid == NOTHREAD)
    return -1;
  PCB* curproc = CURPROC;
  PTCB *ptcb = (PTCB*)tid;
  rlnode* node = rlist_find(&curproc->ptcb_list, ptcb, NULL);//node=ptcb_list_node
  if(node==NULL)
    return -1;
  if(node->ptcb->exited==1){
	  return -1;
  }
  else
  {
    node->ptcb->detached = 1;
    kernel_broadcast(&node->ptcb->exit_cv);
    return 0;
  }
  
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{

  PCB* curproc = CURPROC;
  TCB* curthread = cur_thread();
  



 //COPY FROM EXIT
 if(curproc->thread_count==1){
    /* Reparent any children of the exiting process to the 
       initial task */
    
    rlnode* list = &curthread->owner_pcb->ptcb_list;
  while (rlist_len(list)>0){
    rlist_remove(list->next);
    free(list->next->ptcb);
  }
 


    PCB* initpcb = get_pcb(1);
    while(!is_rlist_empty(& curproc->children_list)) {
      rlnode* child = rlist_pop_front(& curproc->children_list);
      child->pcb->parent = initpcb;
      rlist_push_front(& initpcb->children_list, child);
    }

    /* Add exited children to the initial task's exited list 
       and signal the initial task */
    if(!is_rlist_empty(& curproc->exited_list)) {
      rlist_append(& initpcb->exited_list, &curproc->exited_list);
      kernel_broadcast(& initpcb->child_exit);
    }
    if(get_pid(curproc)!=1){
    /* Put me into my parent's exited list */
    rlist_push_front(& curproc->parent->exited_list, &curproc->exited_node);
    kernel_broadcast(& curproc->parent->child_exit);
    }
  

  assert(is_rlist_empty(& curproc->children_list));
  assert(is_rlist_empty(& curproc->exited_list));


  /* 
    Do all the other cleanup we want here, close files etc. 
   */

  /* Release the args data */
  if(curproc->args) {
    free(curproc->args);
    curproc->args = NULL;
  }

  /* Clean up FIDT */
  for(int i=0;i<MAX_FILEID;i++) {
    if(curproc->FIDT[i] != NULL) {
      FCB_decref(curproc->FIDT[i]);
      curproc->FIDT[i] = NULL;
    }
  }



  
  

  /* Disconnect my main_thread */
  //curproc->main_thread = NULL;

  /* Now, mark the process as exited. */
  curproc->pstate = ZOMBIE;

  /* Bye-bye cruel world */
  kernel_sleep(EXITED, SCHED_USER);
 

 }//END COPY EXIT
 if(curthread->ptcb->detached==1){//we delete ptcb because its detached and there's no way any thread would join it in the future.
  curthread->ptcb->exitval = exitval;
  curthread->ptcb->exited = 1;
  curproc->thread_count--;
  rlist_remove(&curthread->ptcb->ptcb_list_node);
  free(curthread->ptcb);
 }
 else if(curthread->ptcb->refcount !=0){//we dont delete ptcb because maybe another thread wants to join it later and get its exitval.
  curthread->ptcb->exitval = exitval;
  curthread->ptcb->exited = 1;
  curproc->thread_count--;
  kernel_broadcast(&curthread->ptcb->exit_cv);
 }
 else if(curthread->ptcb->refcount == 0){//we dont delete ptcb because maybe another thread wants to join it later and get its exitval.
  curthread->ptcb->exitval = exitval;
  curthread->ptcb->exited = 1;
  curproc->thread_count--;
 }
  /* Bye-bye cruel world */
kernel_sleep(EXITED, SCHED_USER);

}

