#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <queue>
#include <assert.h>
#include "1t.h"
#include "interrupt.h"

using namespace std;

typedef ucontext_t* ucontext_ptr;

struct semV
{
    unsigned int value;
    unsigned int original;
    queue<ucontext_ptr> waitQ;
};

static ucontext_ptr kernelTCB,parentTCB,initTCB;

static queue<ucontext_ptr> readyQ;

static queue<ucontext_ptr> yieldQ;

static ucontext_ptr exeTCB;

static map<unsigned int,semV> sems;

static int fromFlag = 1;
static int initFlag = 0;
void kernel(void);
//void parent(void);
int dthreads_init(dthreads_func_t parent, void *arg)
{
    interrupt_disable();
    assert_interrupts_disabled();

    //re-init error
    if (initFlag == 1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    initTCB= new ucontext_t;
    kernelTCB=new ucontext_t;
    parentTCB=new ucontext_t;
    getcontext(initTCB);
    //create kenerl TCB
    if(getcontext(kernelTCB) == -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    char *kStack = new char[STACK_SIZE];
    kernelTCB->uc_stack.ss_sp = kStack;
    kernelTCB->uc_stack.ss_size =STACK_SIZE;
    kernelTCB->uc_stack.ss_flags = 0;
    kernelTCB->uc_link = NULL;
    makecontext(kernelTCB,kernel,0);
    
    //create parent TCB
    if(getcontext(parentTCB) == -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    char *pStack = new char[STACK_SIZE];
    parentTCB->uc_stack.ss_sp = pStack;
    parentTCB->uc_stack.ss_size =STACK_SIZE;
    parentTCB->uc_stack.ss_flags = 0;
    parentTCB->uc_link = kernelTCB;
    makecontext(parentTCB,(void(*)())parent,0);
    
    interrupt_enable();
    if(swapcontext(initTCB,parentTCB)== -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    exeTCB = NULL;
    fromFlag = 1;  //default from user function
    initFlag = 1;
    return 0;
}

int dthreads_start(dthreads_func_t func, void *arg)
{
  interrupt_disable();
  assert_interrupts_disabled();
  //after-init error
  if (initFlag == 0)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }

  //create function TCB
  ucontext_ptr fTCB;
  fTCB=new ucontext_t;
  if(getcontext(fTCB) == -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
    char *fStack = new char[STACK_SIZE];
    fTCB->uc_stack.ss_sp = fStack;
    fTCB->uc_stack.ss_size =STACK_SIZE;
    fTCB->uc_stack.ss_flags = 0;
    fTCB->uc_link = kernelTCB;
    makecontext(fTCB,(void(*)())func,1,arg);
    readyQ.push(fTCB);
    
    interrupt_enable();
    swapcontext(parentTCB,kernelTCB);
    return 0;
}

int dthreads_yield(void)
{
    interrupt_disable();
    assert_interrupts_disabled();
    //after-init error
    if (initFlag == 0)
      {
	interrupt_enable();
	assert_interrupts_enabled();
	return -1;
      }

    readyQ.push(exeTCB);
    fromFlag = -1;
    swapcontext(exeTCB,kernelTCB);
    return 0;
}

int dthreads_seminit(unsigned int ID,unsigned int value)
{
  interrupt_disable();
  assert_interrupts_disabled();
  //after-init error
  if (initFlag == 0)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
  
  if (sems.find(ID) != sems.end())  //semaphore already exist
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
  sems[ID].value = value;
  sems[ID].original = value;
  return 0;
}

int dthreads_semdown(unsigned int sem)
{
  interrupt_disable();
  assert_interrupts_disabled();
  //after-init error
  if (initFlag == 0)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
  
  if (sems.find(sem) == sems.end()){
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  if (sems[sem].value == 0){
    fromFlag = 0;
    sems[sem].waitQ.push(exeTCB);
    swapcontext(exeTCB, kernelTCB);
  }
  sems[sem].value--;
  interrupt_enable();
  return 0;
}

int dthreads_semup(unsigned int sem)
{
  interrupt_disable();
  assert_interrupts_disabled();
  //after-init error
  if (initFlag == 0)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
  
  if (sems.find(sem) == sems.end() || sems[sem].value == sems[sem].original){
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  if (!sems[sem].waitQ.empty()){
    fromFlag = 0;
    readyQ.push(sems[sem].waitQ.front());
    sems[sem].waitQ.pop();
    swapcontext(exeTCB, kernelTCB);
  }
  sems[sem].value++;
  interrupt_enable();
  return 0;
}

void kernel(void)
{
  //delete parent TC
  interrupt_disable();
  assert_interrupts_disabled();
  //after-init error
  if (initFlag == 0)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      // return -1;
      return;
    }
  delete[](char*) parentTCB->uc_stack.ss_sp;
  
  while (!readyQ.empty())
    {
      if (fromFlag == 1 &&exeTCB != NULL)
	{
	  interrupt_disable();
	  delete[](char*) exeTCB->uc_stack.ss_sp;
	}
      exeTCB = readyQ.front();
      readyQ.pop();
      fromFlag = 1;
      interrupt_enable();
      swap(kernelTCB, exeTCB);
    }
  //exit when empty                                                                        
  delete[] (char*)kernelTCB->uc_stack.ss_sp;
  printf("Thread library exiting.\n");
  exit(0);
}
