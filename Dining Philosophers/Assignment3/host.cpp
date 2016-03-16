/*
* ASSIGNMENT 3
*-------------------------------
* Author : Sai Vempali
* UFID   : 16141381
* email  : vishnu24@ufl.edu
*-------------------------------
* 
* host.cpp
*---------------------------
* The host.cpp file initializes the shared data structures for the
* N philosophers. It initializes four shared data structures. 
* 1. Shared semaphore data structure for the forks(N)
* 2. Shared semaphore data structure for the barrier(1)
* 3. Shared semaphore data structure for the state of each philosopher
* 4. Shared semaphore data structure for all the philosophers together.
* It then creates the N philosophers using "execv" system call and sends
* necessary arguments for the philosopher.
*
*/

#include<iostream>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<fstream>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include "sem.h"

using namespace std;


int main(int argc, char **argv)
{	
	int numPhil = atoi(argv[1]);
	int phil_iterations = atoi(argv[2]);
	int status;

//=======================Fork Semaphore start=====================================

	// host program - creates the semaphore
	semaphore_t *semap[numPhil];

	// creating as many number of semaphores as there are forks
	for(int i=0; i<numPhil; i++)
	{
		string name = "semaphore";
		name = name + "_" + to_string(i);
		char* sem_name = new char[name.length()-1];
		strcpy(sem_name, name.c_str());
		semap[i] = semaphore_create(sem_name, 0);
	}

//=======================Fork Semaphore end=======================================


//=======================Barrier Semaphore start==================================

	// barrier semaphore
	semaphore_t* barrier_map;
	string barrier = "barrier";
	char* b_name = new char[barrier.length()-1];
	strcpy(b_name, barrier.c_str());
	
	barrier_map = semaphore_create(b_name, numPhil);

	if(barrier_map == NULL)
		exit(1);

	semaphore_close(barrier_map);

//=======================Barrier Semaphore end====================================


//=======================Philosopher Semaphore start==============================

	semaphore_t *philosopher_map;
	string philosopher = "philosopherGlobal";

	char *phil_name = new char[philosopher.length()-1];
	strcpy(phil_name, philosopher.c_str());

	philosopher_map = semaphore_create(phil_name, 1);

	if(philosopher_map == NULL)
		exit(1);

	semaphore_close(philosopher_map);

//=======================Philosopher Semaphore end================================


//=======================State Semaphore start====================================

	state_t* stmap;
	string st = "state";

	char *state_name = new char[st.length()-1];
	strcpy(state_name, st.c_str());

	stmap = state_create(state_name, numPhil);

//=======================State Semaphore end======================================



//========================creating N philosophers=================================

	for(int i=0; i<numPhil; i++)
	{
		pid_t pid = fork();
		if(pid == 0)
		{
			// philosopher child
			string name = "semaphore";
			name = name + "_" + to_string(i);
			char *sem = new char[name.length()-1];
			strcpy(sem, name.c_str());
			char arg[20] = "./philosopher";
			
			string id_as_string = to_string(i);
			char *id = new char[id_as_string.length()-1];
			strcpy(id, id_as_string.c_str());
			char *args[] = {arg, sem, b_name, argv[1], argv[2], state_name,id,phil_name, NULL};
			execv(args[0],args);
		}

		else
		{
			continue;
		}
	}

	for(int i=0; i<numPhil; i++)
		wait(&status);

	for(int i=0; i<numPhil; i++)
	{
		string name = "semaphore_";
		name = name + to_string(i);
		char *sem_name = new char[name.length()-1];
		strcpy(sem_name, name.c_str());

		ifstream semaphore_name(sem_name);
		if(semaphore_name)
		{
			remove(sem_name);
			semaphore_name.close();
		}
	}

	ifstream barrier_name(b_name);
	if(barrier_name)
	{
		remove(b_name);
		barrier_name.close();
	}
	
	ifstream philosopher_name(phil_name);
	if(philosopher_name)
	{
		remove(phil_name);
		philosopher_name.close();
	}
	
	ifstream st_name(state_name);
	if(st_name)
	{
		remove(state_name);
		st_name.close();
	}
	return 0;
}

