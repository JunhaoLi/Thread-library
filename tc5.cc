#include <iostream>
#include <assert.h>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include "1t.h"

using namespace std;

void parent(void *arg);
void consumer(void *arg);
void producer(void *arg);

unsigned int MaxSodas = 10;
unsigned int mutex = 1;
unsigned int fullBuffers = 2;
unsigned int emptyBuffers = MaxSodas;

int main(void) 
{

	if(dthreads_init((dthreads_func_t)parent,(void *)NULL)==-1) 
	{
		cout<<"dthreads_init fails"<<endl;
	}
  return 0;
}

void parent(void *arg)
{
		if(dthreads_start((dthreads_func_t)consumer,(void *)1)==-1) 
		{
			cout<<"dthreads_start consumer fails"<<endl;
		}

		if(dthreads_start((dthreads_func_t)producer,(void *)1)==-1)
		{
			cout<<"dthreads_start producer fails"<<endl;
		}
		dthreads_seminit(mutex, 1);
		if(dthreads_seminit(mutex,1)==-1) 
		{
			cout<<"dthreads_seminit mutex fails"<<endl;
		} 
		
		if(dthreads_seminit(fullBuffers,0)==-1) 
		{
			cout<<"dthreads_seminit fullbuffer fails"<<endl;
		}
		
		if (dthreads_seminit(emptyBuffers, MaxSodas) == -1)
		{
			cout<<"dthreads_seminit emptybuffer fails"<<endl;
		}
}

void consumer(void *arg) 
{
	int id = (int)arg;

	if (dthreads_semdown(fullBuffers) == -1)
	{
		cout << "semaphore down fail" << endl;
	}
	cout << "after semaphore fullbuffers down" << fullBuffers << endl;
	
	if (dthreads_semdown(mutex) == -1)
	{
		cout << "semaphore down fail" << endl;
	}
	cout << "after semaphore mutex down" << mutex << endl;
	cout << "consumer"<<id<<"taking soda out" << endl;
	
	if (dthreads_semup(mutex) == -1)
	{
		cout << "semaphore up fail" << endl;
	}
	cout << "after consumer semaphore up" << mutex << endl;
	
	if (dthreads_semup(emptyBuffers) == -1)
	{
		cout << "semaphore up fail" << endl;
	}
	cout << "after consumer semaphore up" << emptyBuffers << endl;
	
	dthreads_yield();
}


void producer(void *arg) 
{
	int id = (int)arg;

	if (dthreads_semdown(emptyBuffers) == -1)
	{
		cout << "semaphore down fail" << endl;
	}
	cout << "after producer emptyBuffers down" << emptyBuffers << endl;
	
	if (dthreads_semdown(mutex) == -1)
	{
		cout << "semaphore down fail" << endl;
	}
	cout << "after producer mutex down" << mutex << endl;

	cout << "producer "<<id<<" putting soda in" << endl;

	if (dthreads_semup(mutex) == -1)
	{
		cout << "semaphore up fail" << endl;
	}
	cout << "after producer  mutex semaphore up" << mutex << endl;
	
	if (dthreads_semup(fullBuffers) == -1)
	{
		cout << "semaphore up fail" << endl;
	}

	cout << "after producer  fullBuffers semaphore up " << fullBuffers << endl;
	dthreads_yield();
}