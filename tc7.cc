/*test case 2 */
/*This should be a right case*/
#include "1t.h"
#include <cstdlib>
#include <iostream>

using namespace std;

unsigned int Q=2;
unsigned int cust=3;
int waitnum = 0;

void cthread();
void customer(void *args);
void panda(void *args);

int main(int argc, char ** argv) {

  if(dthreads_init((dthreads_func_t) cthread, (void *) 10)) {
    cout << "dthreads_init failed!" << endl;
    exit(1);
  }
  return 0;
}


void cthread() {

  if(dthreads_seminit(Q, 0)) {
    cout<<"init sem "<<Q<<" error"<< endl;
    exit(1);
  }
  if(dthreads_seminit(cust, 0)) {
    cout<<"init sem "<<cust<<" error"<< endl;
    exit(1);
  }

  for(int i = 0; i < 5; i++) {
    if(dthreads_start((dthreads_func_t) customer, (void *) i)) {
      cout<<"dthreads_start error"<<endl;
      exit(1);
    } 
  }
  if(dthreads_start((dthreads_func_t) panda, (void *) "panda")) {
    cout<<"dthreads_start error"<<endl;
    exit(1);
  }
  return;
}

void customer(void * args) {
  int tid = (int) args;
  if(tid == 2) {
    if(dthreads_yield()) {
      cout<<"dthreads_yield error"<<endl;
      exit(1);
    }
  }
  cout << "customer " << tid << " waiting" << endl;
  waitnum++;
  //  cout << cust << endl;
  if(dthreads_semup(cust)) {
    cout <<"dthreads_semup error" << endl;
    exit(1);
  }
  // cout<< "cust up" << endl;
  // cout << Q << endl;
  if(dthreads_semdown(Q)) {
    cout<< "dthreads_semdown error" << endl;
    exit(1);
  }
  // cout<< "Q down" << endl;
  waitnum--;
  cout <<"customer " <<tid << " finished" << endl;
}

void panda(void *args) {
   while(1) {
     if(dthreads_semdown(cust)) {
       cout << "dthreads_semdown error" << endl;
       exit(1);
     }
     if(dthreads_semup(Q)) {
       cout << "dthreads_semup error" << endl;
       exit(1);
     }
     cout << "serve one customer" <<endl;
   }
}
