/*
* ASSIGNMENT 3
*-------------------------------
* Author : Sai Vempali
* UFID   : 16141381
* email  : vishnu24@ufl.edu
*-------------------------------
* 
* sem.h
*------------
* The sem.h header file consists of the shared data structures 
* and their definitions. It also declares and defines all the 
* functions that are used by these shared data structures. 
*
*/

#include<pthread.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<fcntl.h>
#include<string>

using namespace std;

//====================================STATE shared data structure Start==============================================

//-------------State Data Strucure-----------------
struct state
{
	pthread_mutex_t state_lock;
	int states[100];
};

typedef struct state state_t;

pthread_mutexattr_t statesharedm;

//-------------State Create------------------------
state_t* state_create(char *state_name, int state_size)
{
	
	state_t *stmap;

	int fd = open(state_name, O_RDWR|O_CREAT|O_EXCL, 0666);
	ftruncate(fd, sizeof(state_t));
	
	pthread_mutexattr_init(&statesharedm);
	pthread_mutexattr_setpshared(&statesharedm, PTHREAD_PROCESS_SHARED);
	
	stmap = (state_t*)mmap(NULL, sizeof(state_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	close(fd);
	pthread_mutex_init(&stmap->state_lock, &statesharedm);

	for(int i=0; i < state_size; i++)
		stmap->states[i] = 0;

	return stmap;	
	
}

//-------------State Open-------------------------------

state_t* state_open(char *state_name)
{
	state_t* stmap;
	int fd = open(state_name, O_RDWR);	
	if(fd < 0)	
		return NULL;
	
	stmap = (state_t*) mmap(NULL, sizeof(state_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	close(fd);
	return stmap;
}

//====================================STATE shared data structure End================================================

//====================================FORKS Semaphore shared Start===================================================

//-------------Semaphore structure--------------------------
struct semaphore 
{
	pthread_mutex_t lock;
        pthread_cond_t nonzero;
        unsigned count;
};
typedef struct semaphore semaphore_t;


semaphore_t *semaphore_create(char *semaphore_name, int count);
semaphore_t *semaphore_open(char *semaphore_name);
void semaphore_post(semaphore_t *semap);
void semaphore_wait(semaphore_t *semap);
void semaphore_close(semaphore_t *semap);

//-------------Semaphore shared attributes--------------------
pthread_mutexattr_t psharedm;
pthread_condattr_t psharedc;

//-------------Semaphore create-------------------------------
semaphore_t *semaphore_create(char *semaphore_name, int count)
{
	semaphore_t *semap;

	int fd = open(semaphore_name, O_RDWR|O_CREAT|O_EXCL, 0666);
	ftruncate(fd, sizeof(semaphore_t));

	pthread_mutexattr_init(&psharedm);
	pthread_condattr_init(&psharedc);

	pthread_mutexattr_setpshared(&psharedm, PTHREAD_PROCESS_SHARED);
	pthread_condattr_setpshared(&psharedc, PTHREAD_PROCESS_SHARED);
	
	semap = (semaphore_t*)mmap(NULL, sizeof(semaphore_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	close(fd);
	
	pthread_mutex_init(&semap->lock, &psharedm);
	pthread_cond_init(&semap->nonzero, &psharedc);
	semap->count = count;

	return semap;	
}

//-------------Semaphore open--------------------------------
semaphore_t *semaphore_open(char *semaphore_name)
{
	semaphore_t* semap;
	int fd = open(semaphore_name, O_RDWR);
	if(fd < 0)	
		return NULL;
	
	semap = (semaphore_t*) mmap(NULL, sizeof(semaphore_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	return semap;
}

//-------------Semaphore up----------------------------------
void semaphore_post(semaphore_t *semap)
{
	pthread_mutex_lock(&semap->lock);
	
	if(semap->count == 0)
		pthread_cond_signal(&semap->nonzero);
	
	semap->count++;
		
	pthread_mutex_unlock(&semap->lock);
}

//-------------Semaphore wait---------------------------------
void semaphore_wait(semaphore_t *semap)
{
	pthread_mutex_lock(&semap->lock);

	if(semap->count == 0)
		pthread_cond_wait(&semap->nonzero, &semap->lock);

	semap->count--;
	
	pthread_mutex_unlock(&semap->lock);

}

//-------------Semaphore close---------------------------------
void semaphore_close(semaphore_t *semap)
{
	munmap((void*)semap, sizeof(semaphore_t));
}

//====================================FORKS Semaphore shared End=====================================================

