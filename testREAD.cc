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
