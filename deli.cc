/*
 * deli.cc
 *
 *  Created on: Feb 1, 2015
 *      Author: junhaoli
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include <iostream>

#include "dthreads.h"
#include "interrupt.h"

using namespace std;

//struct to parent function, stores status information
struct Info
{
	int cashierNum;
	int activeCashier;
	int *corkBoard;
        int *corkID;
	int boardSize;
	int currDish;
        int boundary;
	char **cashierFiles;
};

//global variable Info instance
Info *inf = (Info *)malloc(sizeof(Info));


//function declareations
void parentFunc(void *argu);
void cashierFunc(void *argu);
void makerFunc(void *argu);
int readNextRequest(FILE *fp);
int findNearest(int prev);
int findFirstSlot();
void sweepBoard();



int main(int argc,char **argv)
{
	if (argc<3)
	{
	  //invalid arguments number
	  exit(1);
	}

	inf->cashierNum = argc-2;
	int realSize = (argc-2<atoi(argv[1])?argc-2:atoi(argv[1]));
	inf->corkBoard = (int *)malloc(sizeof(int)*realSize);
	inf->corkID = (int *)malloc(sizeof(int)*realSize);
	inf->activeCashier = argc-2;
	inf->cashierFiles = (char **)malloc(sizeof(char *)*(argc-2));
	inf->boardSize = realSize;
	inf->boundary = realSize;
	inf->currDish = 0;
	memset(inf->corkBoard,-1,sizeof(int)*realSize);
	memset(inf->corkID,-1,sizeof(int)*realSize);
	for (int i = 2; i<argc;i++)
	{
	  inf->cashierFiles[i-2] = argv[i];
	}

	if(dthreads_init((dthreads_func_t)parentFunc,(void *)inf))
	{
	  //parent init failed
	  exit(1);
	}
	//main completed
	return 0;
}

void parentFunc(void *argu)
{
        begin_preemptions(false,true,0);
	Info *para = (Info *)argu;

	for (int i = 0; i<para->cashierNum;i++)
	{
	    int *cashierID = (int *)malloc(sizeof(int));
	    *cashierID = i;
	    if(dthreads_start((dthreads_func_t)cashierFunc,(void *)cashierID))
	    {
	      exit(1);
	    }
	}

	if(dthreads_start((dthreads_func_t)makerFunc,(void *)NULL))
	{
	    exit(1);
	}
}

void cashierFunc(void *argu)
{
	dthreads_lock(0);

	int *ID = (int *)argu;
	FILE *fp = fopen(inf->cashierFiles[*ID],"r");
	int requestNum = readNextRequest(fp);
	
	while (requestNum != -1)
	{
		if (inf->currDish<inf->boardSize)
		{
			int index = findFirstSlot();
			inf->corkBoard[index] = requestNum;
			inf->corkID[index] = *ID;
			inf->currDish++;
			cout <<"POSTED: cashier "<<*ID <<" sandwich "<<requestNum<<endl;
			if (inf->currDish == inf->boardSize)
			{
			  dthreads_signal(0,-1);
			}
			dthreads_wait(0,*ID);
			//cook completed,lock acquired
			requestNum = readNextRequest(fp);
			dthreads_signal(0,-2);
			dthreads_wait(0,-3);
		}
		else
		{
		  dthreads_wait(0,-3);
		}
	}
	inf->activeCashier--;
	if (inf->activeCashier == 0) //last quit, free all memory in Info
	{
	  //
	}
	else
	{
	  if (inf->activeCashier>=inf->boundary)
	  {
	    if (inf->corkID[*ID] == *ID)
	    {
	      inf->corkBoard[*ID] = -1;
	    }
	    
	  }  
	  else
	  {
	    int sl = findFirstSlot();
	    inf->corkBoard[sl] = -2;
	  }
	   inf->boardSize = (inf->activeCashier>inf->boardSize?inf->boardSize:inf->activeCashier);
	   dthreads_signal(0,-1);
	}
	free(ID);
	fclose(fp);
	dthreads_unlock(0);
}

void makerFunc(void *argu)
{
	dthreads_lock(0);
	int previousDish = -1;
	int totalNum = 1;
	while (inf->activeCashier >0)
	{
		while ((inf->currDish)<(inf->boardSize))
		{
		  dthreads_broadcast(0,-3);
		  dthreads_wait(0,-1);
		}
		int index = findNearest(previousDish);
		int requestName = inf->corkBoard[index];
		int cashierID = inf->corkID[index];
		inf->corkBoard[index] = -1;
		inf->corkID[index] = -1;
		inf->currDish--;
		cout <<"READY: cashier "<<cashierID<<" sandwich "<<requestName<<endl;
		totalNum++;
		previousDish = requestName;
		dthreads_signal(0,cashierID);
		dthreads_wait(0,-2);
		dthreads_broadcast(0,-3);
		dthreads_wait(0,-1);
	}
	dthreads_unlock(0);
}


int readNextRequest(FILE *fp)
{
	char *myLine = NULL;
	size_t len = 0;
	if (getline(&myLine,&len,fp) != -1)
	{

		int val = atoi(myLine);
		free(myLine);
		return val;
	}
	else
	{
		free(myLine);
		return -1;
	}
}

int findNearest(int prev)
{

	int index = -1;
	int diff = -1;
	for (int i = 0; i<inf->boundary;i++)
	{
	  if (inf->corkBoard[i] != -2)
	  {
	    index = i;
	    diff = abs(prev-inf->corkBoard[i]);
	    break;
	  }
	}

	for (int i = index+1; i<inf->boundary;i++)
	{
	  if (inf->corkBoard[i] == -2)
	    {
	      continue;
	    }
		int curr = abs(prev-inf->corkBoard[i]);
		index = (diff<curr?index:i);
		diff = (diff<curr?diff:curr);
	}
	return index;
}

int findFirstSlot()
{
	for (int i = 0; i<inf->boundary;i++)
	{
	   if (inf->corkBoard[i] == -1)
	   {
	      return i;
	   }
	}
	return -1;
}
