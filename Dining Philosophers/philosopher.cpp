/*
* ASSIGNMENT 3
*-------------------------------
* Author : Sai Vempali
* UFID   : 16141381
* email  : vishnu24@ufl.edu
*-------------------------------
* 
* philosopher.cpp
*---------------------------
* The philosopher.cpp implements a philosopher that sits a loop
* which executes M times(M is passed as an argument) and each time
* the philosopher prints its current state to the terminal.
*
*/


#include<iostream>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include "sem.h"

using namespace std;

void think(int id, state_t* state_map)
{
	state_map->states[id] = 0;
	cout<<"Philosopher "<<id<<" is THINKING\n";
}

void test(int id, semaphore_t* semap, state_t* state_map, int N)
{
	int left = (id+N-1) % N;
	int right = (id+1) % N;
	
	pthread_mutex_lock(&state_map->state_lock);

	int currPhil = state_map->states[id];
	int leftPhil = state_map->states[left];
	int rightPhil = state_map->states[right];

	pthread_mutex_unlock(&state_map->state_lock);
	
	if(currPhil == 1 && leftPhil != 2 && rightPhil != 2)
	{
		pthread_mutex_lock(&state_map->state_lock);
		state_map->states[id] = 2;
		pthread_mutex_unlock(&state_map->state_lock);
		semaphore_post(semap);
	}
}


void take_fork(int id, semaphore_t* semap, state_t* state_map, semaphore_t* philGlobal, int N)	
{
	semaphore_wait(philGlobal);

	pthread_mutex_lock(&state_map->state_lock);
	state_map->states[id] = 1;
	pthread_mutex_unlock(&state_map->state_lock);

	cout<<"Philosopher "<<id<<" is HUNGRY\n";
	test(id, semap, state_map, N);

	semaphore_post(philGlobal);
	semaphore_wait(semap);
}

void eat(int id)
{
	cout<<"Philosopher "<<id<<" is EATING\n";
}

void put_fork(int id, state_t* state_map, semaphore_t* philGlobal, int N)
{
	semaphore_wait(philGlobal);

	pthread_mutex_lock(&state_map->state_lock);
	state_map->states[id] = 0;
	pthread_mutex_unlock(&state_map->state_lock);

	int left = (id+N-1) % N;
	int right = (id+1) % N;
	
	string leftname = "semaphore_";
	leftname = leftname + to_string(left);
	char *lname = new char[leftname.length()-1];
	strcpy(lname, leftname.c_str());
	semaphore_t *semapLeft = semaphore_open(lname);

	string rightname = "semaphore_";
	rightname = rightname + to_string(right);
	char *rname = new char[rightname.length()-1];
	strcpy(rname, rightname.c_str());
	semaphore_t *semapRight = semaphore_open(rname);
	
	test(left, semapLeft, state_map, N);

	test(right, semapRight, state_map, N);

	semaphore_post(philGlobal);
}

int main(int argc, char **argv)
{

	char *sem_name = argv[1];

	char *barrier = argv[2];

	// N = No of philosophers(=forks)
	int N = atoi(argv[3]);

	// M = No of times a philosopher runs
	int M = atoi(argv[4]);

	char *state_name = argv[5];
	
	int id = atoi(argv[6]);
	cout<<"Philosopher "<<id<<" started\n";
	
	char *phil_name = argv[7];
//================================barrier start================================================

	semaphore_t *b_map = semaphore_open(barrier);
	pthread_mutex_lock(&b_map->lock);
	b_map->count--;
	if(b_map->count != 0)
	{
		cout<<"Philosopher "<<id<<" is waiting at barrier point\n";
		pthread_cond_wait(&b_map->nonzero, &b_map->lock);
		pthread_mutex_unlock(&b_map->lock);		
	}
	
	else
	{
		cout<<"Philosopher "<<id<<" is the final process\nSignalling other processes to start\n";
		pthread_mutex_unlock(&b_map->lock);		
		pthread_cond_broadcast(&b_map->nonzero);
	}
	
//================================barrier end================================================

//================================Philosopher start==========================================

	state_t *state_map = state_open(state_name);
	if(state_map == NULL)
		cout<<"state failed\n";

	semaphore_t *philGlobal = semaphore_open(phil_name);
	semaphore_t *semap = semaphore_open(sem_name);

	for(int i=0; i<M; i++)	
	{
		think(id, state_map);
		take_fork(id, semap, state_map, philGlobal, N);
		eat(id);
		put_fork(id, state_map, philGlobal, N);
		think(id, state_map);
		
	}

//================================Philosopher end===============================================
	cout<<"Philosopher "<<id<<" is terminating\n";
	return 0;
} 
