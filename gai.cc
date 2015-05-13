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

static ucontext_ptr kernelTCB,parentTCB,initTCB, fTCB, runningTCB;

static queue<ucontext_ptr> readyQ;

static queue<ucontext_ptr> yieldQ;

static ucontext_ptr exeTCB;

static map<unsigned int,semV> sems;

static int fromFlag = 1;
static int initFlag = 0;
int kernel(dthreads_func_t func, void *arg);
void work(dthreads_func_t func, void *arg);
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
    //parentTCB=new ucontext_t;
    // getcontext(initTCB);
if(getcontext(initTCB) == -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
/* char *iStack = new char[STACK_SIZE];
    initTCB->uc_stack.ss_sp = iStack;
    initTCB->uc_stack.ss_size =STACK_SIZE;
    initTCB->uc_stack.ss_flags = 0;
    initTCB->uc_link = NULL;
    makecontext(initTCB,(void(*)())dthreads_init,0);
*/      
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
    makecontext(kernelTCB,(void(*)())kernel,2,parent,arg);
    
    //create parent TCB
    /*if(getcontext(parentTCB) == -1)
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
    exeTCB=parentTCB;
    interrupt_enable();
    fromFlag=1;
    initFlag=1;
    if(swapcontext(initTCB,parentTCB)== -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
      }*/
    exeTCB = NULL;
    fromFlag = 1;  //default from user function
    initFlag = 1;
    interrupt_enable();
    // parent(arg);
    runningTCB=kernelTCB;
    swapcontext(initTCB,kernelTCB);
    //swapcontext(initTCB,kernelTCB);
    //kernel();
    return 0;
}
void work(dthreads_func_t func, void *arg){
  //interrupt_enable();
  func(arg);
  swapcontext(exeTCB,kernelTCB);
}
int dthreads_start(dthreads_func_t func, void *arg)
{
  //printf("start\n");
  interrupt_disable();
  //printf("s1\n");
  assert_interrupts_disabled();
  //after-init error
  //printf("jinru start\n");
  if (initFlag == 0)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }

  //create function TCB
  
  fTCB=new ucontext_t;
  //printf("get zhiqian\n");
  if(getcontext(fTCB) == -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
  //printf("get zhihou\n");
    char *fStack = new char[STACK_SIZE];
    fTCB->uc_stack.ss_sp = fStack;
    fTCB->uc_stack.ss_size =STACK_SIZE;
    fTCB->uc_stack.ss_flags = 0;
    fTCB->uc_link = NULL;
    makecontext(fTCB,(void(*)())work,2,func,arg);
    readyQ.push(fTCB);
    //printf("push zhihou\n");  
    interrupt_enable();
  //    swapcontext(parentTCB,kernelTCB);
    return 0;
}
int kernel(dthreads_func_t parent, void *arg)
{
  //delete parent TC
  interrupt_disable();
  assert_interrupts_disabled();
  //after-init error
  //printf("kernel");
  if (initFlag == 0)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      // return -1;
      return -1;
    }
  //delete[](char*) parentTCB->uc_stack.ss_sp;
  //printf("readyQ\n");
  parent(arg);
  while (!readyQ.empty())
    {//printf("wwwwwwwwwwwwwwwww\n");
      if (fromFlag == 1 &&exeTCB != NULL)
	{
	  // interrupt_disable();
	  delete[](char*) exeTCB->uc_stack.ss_sp;
	}
      exeTCB = readyQ.front();
      readyQ.pop();
      fromFlag = 1;
      runningTCB=exeTCB;
      interrupt_enable();
      swapcontext(kernelTCB, exeTCB);
    }
  //printf("ttttttttttttt\n");
  //exit when empty                                                                        
  delete[] (char*)kernelTCB->uc_stack.ss_sp;
  printf("Thread library exiting.\n");
  interrupt_enable();
  swapcontext(kernelTCB,initTCB);
  return 0;
  //exit(0);
  //swapcontext(kernelTCB,initTCB);
}

int dthreads_yield(void)
{
  //printf("yield\n");
  interrupt_disable();
  assert_interrupts_disabled();
  //printf("interrupy\n");  
  //after-init error
    if (initFlag == 0)
      {
	interrupt_enable();
	assert_interrupts_enabled();
	return -1;
      }

    readyQ.push(exeTCB);
    fromFlag = -1;
    //printf("swap\n");
    interrupt_enable();
    //printf("enable\n");
    swap(runningTCB,initTCB);
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
