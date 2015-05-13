#include <stdlib.h>
#include <iostream>
#include "1t.h"
#include <assert.h>
#include <stdio.h>
#include <ucontext.h>

using namespace std;
//semaphores:
unsigned int mutex=0;
unsigned int Duke=1;
unsigned int UNC=2;
//condition variables:
unsigned int DukeUse=0;
unsigned int DukeWait=0;
unsigned int UNCUse=0;
unsigned int UNCWait=0;
void DukeEnter(){
  //while(1){
  
  dthreads_semdown(mutex);
  while(UNCWait>0||UNCUse>0){
    DukeWait++;
    dthreads_semup(mutex);
    dthreads_semdown(Duke);
    dthreads_semdown(mutex);
    if(DukeWait>0){
      DukeWait--;
    }
    if(UNCUse==0){
      dthreads_semup(Duke);
      break;
    }
  }
  DukeUse++;
  dthreads_semup(Duke);
  std::cout<<"now entering DukeEnter***********************\n";
  std::cout<<"DukeUse= "<<DukeUse<<std::endl;
  std::cout<<"DukeWait= "<<DukeWait<<std::endl;
  std::cout<<"UNCUse"<<UNCUse<<std::endl;
  std::cout<<"UNCWait= "<<UNCWait<<std::endl;
  
  dthreads_semup(mutex);
  
}


void DukeLeave(){
  dthreads_semdown(mutex);
  DukeUse--;
  dthreads_semdown(Duke);
  // dthreads_yield();
   
  if(DukeUse==0){
    std::cout<<"it is unc's turn\n";
    while(UNCWait!=0){
      dthreads_semup(UNC);
      UNCWait--;
    }
  }
  std::cout<<"now entering DukeLeave**********************\n";
  std::cout<<"DukeUse= "<<DukeUse<<std::endl;
  std::cout<<"DukeWait= "<<DukeWait<<std::endl;
  std::cout<<"UNCUse"<<UNCUse<<std::endl;
  std::cout<<"UNCWait= "<<UNCWait<<std::endl;
  
  dthreads_semup(mutex);
}

void UNCEnter(){
  dthreads_semdown(mutex);
  while(DukeWait>0||DukeUse>0){
    UNCWait++;
    dthreads_semup(mutex);
    //    dthreads_yield();
    dthreads_semdown(UNC);
    dthreads_semdown(mutex);
    if(UNCWait>0){
      UNCWait--;
    }
    if(DukeUse==0){
      dthreads_semup(UNC);
      break;
    }
    std::cout<<"i was stucked before\n";
  }
  UNCUse++;
  dthreads_semup(UNC);
  std::cout<<"now entering UNCEnter***************************\n";
  std::cout<<"UNCUse"<<UNCUse<<std::endl;
  std::cout<<"UNCWait= "<<UNCWait<<std::endl;
  std::cout<<"DukeUse= "<<DukeUse<<std::endl;
  std::cout<<"DukeWait= "<<DukeWait<<std::endl;
  
  dthreads_semup(mutex);
}


void UNCLeave(){
  dthreads_semdown(UNC);
  dthreads_semdown(mutex);
  UNCUse--;
  if(UNCUse==0){
    std::cout<<"it is Duke's turn\n";
    while(DukeWait!=0){
      dthreads_semup(Duke);
      DukeWait--;
    }
  }
  std::cout<<"now entering UNCLeave*************************\n";
  std::cout<<"UNCUse= "<<UNCUse<<std::endl;
  std::cout<<"UNCWait= "<<UNCWait<<std::endl;
  std::cout<<"DukeUse= "<<DukeUse<<std::endl;
  std::cout<<"DukeWait= "<<DukeWait<<std::endl;
  
  dthreads_semup(mutex);

}

void parent(){
  dthreads_seminit(mutex,1);
  dthreads_seminit(Duke,0);
  dthreads_seminit(UNC,0);
  dthreads_start((dthreads_func_t)DukeEnter,NULL);
  
  dthreads_start((dthreads_func_t)UNCEnter,NULL);
    dthreads_start((dthreads_func_t)UNCEnter,NULL);
  
   dthreads_start((dthreads_func_t)DukeEnter,NULL);
   dthreads_start((dthreads_func_t)DukeEnter,NULL);
  dthreads_start((dthreads_func_t)DukeEnter,NULL);
  
    dthreads_start((dthreads_func_t)DukeLeave,NULL);
   dthreads_start((dthreads_func_t)DukeLeave,NULL);
  dthreads_start((dthreads_func_t)DukeLeave,NULL);
  dthreads_start((dthreads_func_t)DukeLeave,NULL);
   
   dthreads_start((dthreads_func_t)UNCEnter,NULL);
  dthreads_start((dthreads_func_t)UNCEnter,NULL);

    dthreads_start((dthreads_func_t)DukeEnter,NULL);
   dthreads_start((dthreads_func_t)DukeEnter,NULL);
   dthreads_start((dthreads_func_t)DukeEnter,NULL);
   dthreads_start((dthreads_func_t)DukeEnter,NULL);
  
   dthreads_start((dthreads_func_t)UNCEnter,NULL);
  dthreads_start((dthreads_func_t)UNCEnter,NULL);
  
   dthreads_start((dthreads_func_t)UNCLeave,NULL);
   dthreads_start((dthreads_func_t)UNCLeave,NULL);
   dthreads_start((dthreads_func_t)UNCLeave,NULL);
  dthreads_start((dthreads_func_t)UNCLeave,NULL);
  dthreads_start((dthreads_func_t)UNCLeave,NULL);
  
  
}

int main(){
  if(dthreads_init((dthreads_func_t)parent, NULL)){
    cout<<"dthreads_init failed\n";
    exit(1);
  }
}
