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

class semV {
public: 
  unsigned int value;
  queue<ucontext_ptr> waitQ;
  semV(int v)
  {
    value = v;
  }
};

static ucontext_ptr parentTCB,initTCB;

static queue<ucontext_ptr> readyQ;

static ucontext_ptr exeTCB;

static map<unsigned int,semV *> sems;

static int initFlag = 0;

void kernel(void);
void work(dthreads_func_t func, void *arg);

int dthreads_init(dthreads_func_t parent, void *arg)
{
  interrupt_disable();
  assert_interrupts_disabled();

  //check re-init error
  if (initFlag == 1)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  initFlag = 1;  //only init one time



//create initTCB
  try
  {
    initTCB= new ucontext_t;
    if(getcontext(initTCB) == -1)
    {
      interrupt_enable();
      assert_interrupts_enabled();
      return -1;
    }
  }
  catch (bad_alloc &exception)
  {
    delete initTCB;
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

//create parentTCB
  try
  {
    parentTCB=new ucontext_t;
    parentTCB->uc_stack.ss_sp = NULL;
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
    parentTCB->uc_link = NULL;
    makecontext(parentTCB,(void(*)())work,2,parent,arg);
  }
  catch (bad_alloc &exception)
  {
    if(parentTCB->uc_stack.ss_sp != NULL)  //already assigned to charstack
    {
      delete[] (char *)parentTCB->uc_stack.ss_sp;
    }
    parentTCB->uc_stack.ss_sp = NULL;
    delete parentTCB;
    //also need to delete initTCB
    delete initTCB;
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  
  readyQ.push(parentTCB);

  //kernel part
  while (!readyQ.empty())
  {
    //pick up the thread in front of the queue
    exeTCB = readyQ.front();
    readyQ.pop();
    swapcontext(initTCB, exeTCB);

    //check if the thread is complete
    if (exeTCB->uc_stack.ss_flags == 1)
    {
      try
      {
        delete[] (char*)exeTCB->uc_stack.ss_sp;
        exeTCB->uc_stack.ss_sp = NULL;
        delete exeTCB;   
      }
      catch (bad_alloc &exception)
      {
        interrupt_enable();
        assert_interrupts_enabled();
        return -1;
      }
    }
  }
  
  printf("Thread library exiting.\n");
  interrupt_enable();
  assert_interrupts_enabled();
  exit(0);
}


int dthreads_start(dthreads_func_t func, void *arg)
{
  interrupt_disable();
  assert_interrupts_disabled();

  //call before initialize
  if (initFlag == 0)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  
  //create function TCB
  ucontext_ptr fTCB;
  try
  {
    fTCB = new ucontext_t;
    fTCB->uc_stack.ss_sp = NULL;
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
    fTCB->uc_link = NULL;
    makecontext(fTCB,(void(*)())work,2,func,arg);
  }
  catch(bad_alloc &exception)
  {
    if(fTCB->uc_stack.ss_sp != NULL)  //already assigned to charstack
    {
      delete[] (char *)fTCB->uc_stack.ss_sp;
    }
    fTCB->uc_stack.ss_sp = NULL;
    delete fTCB;
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  readyQ.push(fTCB);   

  interrupt_enable();
  assert_interrupts_enabled();
  return 0;
}

int dthreads_yield(void)
{
  interrupt_disable();
  assert_interrupts_disabled();  

  //call before initialze
  if (initFlag == 0)
  {
    interrupt_enable();
    assert_interrupts_enabled();
	  return -1;
  }

  //re-push into ready Q
  readyQ.push(exeTCB);
  swapcontext(exeTCB,initTCB);

  //prepare to return to user function
  interrupt_enable();
  assert_interrupts_enabled();    
  return 0;
}


int dthreads_seminit(unsigned int ID,unsigned int value)
{
  interrupt_disable();
  assert_interrupts_disabled();

  //before-init error
  if (initFlag == 0)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  //check if ID already exists
  if (sems.find(ID) != sems.end())
  {   
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  
  sems[ID]=new semV(value);
  sems[ID]->value = value;
  
  interrupt_enable();
  assert_interrupts_enabled();
  return 0;
}


int dthreads_semdown(unsigned int ID)
{
  interrupt_disable();
  assert_interrupts_disabled();
  
  //call before initialize
  if(initFlag == 0)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  
  //check if ID exists in semaphore
  if(sems.find(ID) == sems.end())
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  
  if (sems[ID]->value > 0)
  {
    sems[ID]->value--;
    interrupt_enable();
    assert_interrupts_enabled();   
    return 0;     
  }
  else
  {
    sems[ID]->waitQ.push(exeTCB);
    swapcontext(exeTCB, initTCB);
  }
  
  interrupt_enable();
  assert_interrupts_enabled();  
  return 0;
}


int dthreads_semup(unsigned int ID)
{
  interrupt_disable();
  assert_interrupts_disabled();

  //call before initialize
  if (initFlag == 0)
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }
  
  //check if ID exists in semaphore
  if (sems.find(ID) == sems.end())
  {
    interrupt_enable();
    assert_interrupts_enabled();
    return -1;
  }

  sems[ID]->value++;
  //check if threads already in waiting queue
  if (!sems[ID]->waitQ.empty())
  {
    readyQ.push(sems[ID]->waitQ.front());
    sems[ID]->waitQ.pop();
    sems[ID]->value--;    
  }
  
  interrupt_enable();
  assert_interrupts_enabled();  
  return 0;
}

//delegate function
void work(dthreads_func_t func, void *arg)
{
  interrupt_enable();
  assert_interrupts_enabled(); 
  func(arg);
  interrupt_disable();
  assert_interrupts_disabled();
  exeTCB->uc_stack.ss_flags = 1;  //need free
  swapcontext(exeTCB,initTCB);
}
