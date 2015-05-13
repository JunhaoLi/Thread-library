#include <iostream>
#include "1t.h"
#include <assert.h>
#include <cstdlib>
#include "1t.cc"
using namespace std;
static int N=4;
void parent(void *arg);
void consumer(void *arg);
void producer(void *arg);
int main(void) {
  // cout<<"main starts"<<endl;
  if(dthreads_init((dthreads_func_t)parent,(void *)NULL)==-1) {
    cout<<"dthreads_init fails"<<endl;
  }
  return 0;
}
void parent(void *arg){
  // cout<<"parent functions works"<<endl;
  for(int i=0;i<5;i++) {
    if(dthreads_start((dthreads_func_t)consumer,(void *)i)==-1) {
     cout<<"dthreads_start consumer fails"<<endl;
    }
  }
  if(dthreads_start((dthreads_func_t)producer,(void *)NULL)==-1) {
    cout<<"dthreads_start producer fails"<<endl;
  }
  if(dthreads_seminit(100,1)==-1) {
    cout<<"dthreads_seminit lock 100 fails"<<endl;
  } 
  if(dthreads_seminit(200,0)==-1) {
    cout<<"dthreads_seminit fullbuffer 200 fails"<<endl;
  } 
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
