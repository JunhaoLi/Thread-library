/* Semaphore test case----duke & unc  use restroom  */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "1t.h"
#include <cstdlib>
#include <assert.h>
//#include "1t.cc"
using namespace std;
//semophore
unsigned int bathroom=1;//1
unsigned int duke=2;//0
unsigned int unc=3;//0

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
  for(int i=0;i<2;i++) { 
    if(dthreads_start((dthreads_func_t)unc_enter, (void*)NULL) == -1){
      cout << "dthreads_start unc_enter fails." << endl;
    }
  }
  /* if(dthreads_start((dthreads_func_t)unc_enter, (void*)NULL) == -1){
    cout << "dthreads_start unc_enter fails." << endl;
    }*/
  if(dthreads_start((dthreads_func_t)duke_enter, (void*)NULL) == -1){
    cout << "dthreads_start duke_enter fails."<< endl;
  }
  for(int i=0;i<2;i++) {
    if(dthreads_start((dthreads_func_t)unc_leaves, (void*)NULL) == -1){
      cout << "dthreads_start unc_leaves fails." << endl;
    }
  }
  if(dthreads_start((dthreads_func_t)duke_leaves, (void*)NULL)== -1){
    cout << "dthreads_start duke_leaves fails." << endl;
  }
  if(dthreads_start((dthreads_func_t)unc_enter, (void*)NULL) == -1){
    cout << "dthreads_start unc_enter fails." << endl;
  }
  /* for(int i=0;i<2;i++) {
    if(dthreads_start((dthreads_func_t)duke_enter, (void*)NULL) == -1){
      cout << "dthreads_start duke_enter fails."<< endl;
    }
  }
  if(dthreads_start((dthreads_func_t)unc_enter, (void*)NULL) == -1){
    cout << "dthreads_start unc_enter fails." << endl;
    }*/
  //don't know whether should we initiate the unc_leaves and duke_leaves
  if(dthreads_start((dthreads_func_t)unc_leaves, (void*)NULL) == -1){
    cout << "dthreads_start unc_leaves fails." << endl;
  }
  /*
  if(dthreads_start((dthreads_func_t)duke_leaves, (void*)NULL)== -1){
    cout << "dthreads_start duke_leaves fails." << endl;
  }
  */
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
  cout<<"after unc_enter semdown\n"<<endl;
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
  cout<<"after ++\n"<<endl;
  dthreads_semup(bathroom);
  cout<<"after unc_enter up\n"<<endl;
}

void unc_leaves(){
  dthreads_semdown(bathroom);
  cout<<"after unc_leaves semdown\n"<<endl; 
 unc_use--;
  if(unc_use == 0){
    //if you're the last UNC fan, let the Duke fans in
    for(int i = 0; i < duke_wait; i++){
      dthreads_semup(duke);
    }
  }
  dthreads_semup(bathroom);
  cout<<"after unc_leaves semup"<<endl;
}

void duke_enter(){
  dthreads_semdown(bathroom);
  cout<<"after duke_enter semdown"<<endl; 
 while((unc_use > 0) || (unc_wait > 0)){
    duke_wait++;
    dthreads_semup(bathroom);
    //block untill Duke fans can go in
 
    dthreads_semdown(duke);
    dthreads_semdown(bathroom);
    duke_wait--;
   printf("dddddddddddddddd\n"); 
   dthreads_yield();
  }
  duke_use++;
  dthreads_semup(bathroom);
  cout<<"after duke_enter semup"<<endl;
}

void duke_leaves(){
  dthreads_semdown(bathroom);
  cout<<"after duke_leaves semdown"<<endl;
  duke_use--;
  if(duke_use == 0){
    //if you're the last Duke fan, let the UNC fans in
    for(int i = 0; i < unc_wait; i++){
      dthreads_semup(unc);
    }
    dthreads_yield();
  }
  dthreads_semup(bathroom);
  cout<<"after duke_leaves semdown"<<endl;
}


/*
#include <iostream>
#include <assert.h>
#include "1t.h"

#include <stdio.h>
#include <stdlib.h>


using namespace std;

int g=0;

void loop(void *a) {
  char *id;
  int i;

  id = (char *) a;
  cout <<"loop called with id " << (char *) id << endl;

  for (i=0; i<5; i++, g++) {
    cout << id << ":\t" << i << "\t" <<
    g << endl;
    if (dthreads_yield()) {
      cout << "dthreads_yield failed\n";
      exit(1);
    }
  }
}

void parent(void *a) {
  int arg;
  arg = (int) a;
  cout << "parent called with arg " << arg << endl;
  if (dthreads_start((dthreads_func_t) loop, 
                     (void *) "child thread")) {
    cout << "dthreads_start failed\n";
    exit(1);
  }

  loop( (void *) "parent thread");
}

int main() {
  if (dthreads_init( (dthreads_func_t) parent, (void *) 100)) {
    cout << "dthreads_init failed\n";
    exit(1);
  }
}
*/
/*
#include <iostream>
#include "1t.h"
#include <assert.h>
#include <cstdlib>
#include<stdio.h>
//#include "1t.cc"
using namespace std;
static int N=4;
void parent(void *arg);
void consumer(void *arg);
void producer(void *arg);
int main(void) {
  //cout<<"main starts"<<endl;
  //  printf("main\n");  
if(dthreads_init((dthreads_func_t)parent,(void *)NULL)==-1) {
    cout<<"dthreads_init fails"<<endl;
  }
  return 0;
}
void parent(void *arg){
  //cout<<"parent functions works"<<endl;
  for(int i=0;i<5;i++) {
    if(dthreads_start((dthreads_func_t)consumer,(void *)i)==-1) {
     cout<<"dthreads_start consumer fails"<<endl;
    }
  }
  if(dthreads_start((dthreads_func_t)producer,(void *)NULL)==-1) {
    cout<<"dthreads_start producer fails"<<endl;
  }
  //cout<<"semini   zhiqian\n"<<endl;
  if(dthreads_seminit(100,1)==-1) {
    cout<<"dthreads_seminit lock 100 fails"<<endl;
  } 
  //cout<<"sem   zhihou\n"<<endl;
  if(dthreads_seminit(200,0)==-1) {
    cout<<"dthreads_seminit fullbuffer 200 fails"<<endl;
  } 
  // cout<<"sem 200\n"<<endl;
   if(dthreads_seminit(300,5)==-1) {
     cout<<"dthreads_seminit emptybuffer 300 fails"<<endl;
   } 
   //cout<<"parent finishes"<<endl;
}

void consumer(void *arg) {
  int i=(int )arg;
  int j=5;
  while(j>0){
  cout<<"consumer"<<i<<" working1"<<endl;
  dthreads_semdown(200);
  // cout<<"consumer working2"<<endl;
  dthreads_semdown(100);
  cout<<"consumer"<<i<<"taking soda out"<<endl;
  //cout<<"N  before --  "<<N<<endl;
  // N--;
  dthreads_semup(100);
  // cout<<"consumer working2"<<endl;
  dthreads_semup(300);
  dthreads_yield();
  }
  // cout<<"consumer finishes"<<endl;
}
void producer(void *arg) {
  int j=5;
  while(j>0){
    cout<<"producer working"<<endl;
    dthreads_semdown(300);
    dthreads_semdown(100);
    cout<<"putting soda in"<<endl;
    dthreads_semup(100);
    // while(N<5) {
    dthreads_semup(200);
      // cout<<"N  before ++  "<<N<<endl;
      // N++;
      // }
      j--;
    dthreads_yield();
  }
  //cout<<"producer finishes"<<endl;
}

*/
