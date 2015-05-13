/* Semaphore test case----duke & unc  use restroom  */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "1t.h"
#include <cstdlib>
#include <assert.h>
#include "1t.cc"
using namespace std;
//semophore
unsigned int bathroom;//1
unsigned int duke;//0
unsigned int unc;//0

int unc_use = 0;
int unc_wait = 0;
int duke_use = 0;
int duke_wait = 0;

void parent(void *arg);
void duke_enter();
void duke_leaves();
void unc_enter();
void unc_leaves();

int main(void){
  if(dthreads_init((dthreads_func_t)parent, (void*)NULL) == -1){
    cout << "dthreads_init fails." << endl;
  }
  return 0;
}

void parent(void* arg){
  if(dthreads_start((dthreads_func_t)unc_enter, (void*)NULL) == -1){
    cout << "dthreads_start unc_enter fails." << endl;
  }
  if(dthreads_start((dthreads_func_t)duke_enter, (void*)NULL) == -1){
    cout << "dthreads_start duke_enter fails."<< endl;
  }
  //don't know whether should we initiate the unc_leaves and duke_leaves
  if(dthreads_start((dthreads_func_t)unc_leaves, (void*)NULL) == -1){
    cout << "dthreads_start unc_leaves fails." << endl;
  }
  if(dthreads_start((dthreads_func_t)duke_leaves, (void*)NULL)== -1){
    cout << "dthreads_start duke_leaves fails." << endl;
  }

  if(dthreads_seminit(bathroom, 1) == -1){
    cout << "dthreads_seminit bathroom fails." << endl;
  }
  if(dthreads_seminit(duke, 0) == -1){
    cout << "dthreads_seminit duke fails." << endl;
  }
  if(dthreads_seminit(unc, 0) == -1){
  cout << "dthreads_seminit unc fails." << endl;
  }
}

void unc_enter(){
  cout << "unc_enter woking"<<endl;
  dthreads_semdown(bathroom);
  while((duke_use > 0) || (duke_wait > 0)){
    unc_wait++;
    dthreads_semup(bathroom);
    cout << "unc fans go in"<<endl;
    //block until UNC fans can go in
    dthreads_semdown(unc);
    dthreads_semdown(bathroom);
    unc_wait--;
    dthreads_yield();
  }
  unc_use++;
  dthreads_semup(bathroom);
}

void unc_leaves(){
  dthreads_semdown(bathroom);
  unc_use--;
  if(unc_use == 0){
    //if you're the last UNC fan, let the Duke fans in
    for(int i = 0; i < duke_wait; i++){
      dthreads_semup(duke);
    }
  }
  dthreads_semup(bathroom);
}

void duke_enter(){
  dthreads_semdown(bathroom);
  while((unc_use > 0) || (unc_wait > 0)){
    duke_wait++;
    dthreads_semup(bathroom);
    //block untill Duke fans can go in
    dthreads_semdown(duke);
    dthreads_semdown(bathroom);
    duke_wait--;
    dthreads_yield();
  }
  duke_use++;
  dthreads_semup(bathroom);
}

void duke_leaves(){
  dthreads_semdown(bathroom);
  duke_use--;
  if(duke_use == 0){
    //if you're the last Duke fan, let the UNC fans in
    for(int i = 0; i < unc_wait; i++){
      dthreads_semup(unc);
    }
    dthreads_yield();
  }
  dthreads_semup(bathroom);
}
