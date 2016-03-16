/*
*combiner.c
*-------------------------------
* Author : Sai Vempali
* UFID   : 16141381
* email  : vishnu24@ufl.edu
*-------------------------------
*
* Combiner reads the input from a text file(file.txt in this case)
*
* It creates two child processes, one child process calls the mapper
* and the other child process calls reducer. This is achieved by using
* the execv() system call.
* 
* The communication between the two child processes is achieved using 
* pipe() and dup2() system calls.
*
* Pipe() : This system creates a pipe and a copy of pipe is present in 
*	   all the 3 processes,i.e, parent,child1 and child2.
*
* dup2() : The dup2() in child 1 overwrites the standard output of the 
*	   the child 1 process with the write end of the pipe so that 
*          the mapper program called by child 1 writes to the write end 
* 	   of the pipe. The read end here is closed.
*
*	   The dup2() in child 2 overwrites the standard input of the 
*	   child 2 process with the read end of the pipe so that reducer
*	   program called by child 2 reads from the read end of the pipe. 
*          The write end here is closed.
*
*	   The read end of the pipe in the parent is closed after reducer
*          finishes execution and write end of the pipe after the mapper 
*	   finsihes execution.
*
* The child2(calls reducer) process is made to wait until child1(calls mapper) 
* is completed so that we do not encounter with error of reducer being executed
* before even the mapper is executed.
*
*/



#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<stdlib.h>

int main(int argc, char *argv[]){

	if(argc < 2){
		printf("Incorrect input entered. Re-enter with correct number of arguments\n");
		exit(0);
	}

	else{
	
		/*parent creates pipe*/
		int fd[2];
		pipe(fd);	

		/*parent creates first child process*/
		pid_t pid1 = fork(); 	
		int status;

		if(pid1 == -1){
			perror("fork for the first child failed\n");
			return -1;
		}

		else if(pid1 == 0){	
		
			/*child 1 - deals with mapper.c*/
		
			char *argsch1[] = {"./mapper",argv[1],NULL};
			close(fd[0]);
			dup2(fd[1],1);	/*dup2 overwrites the standard output of child 1 process to the write end of the pipe */
			execv(argsch1[0],argsch1);

		}

		else{	
			/*parent creates another second child process*/

			pid_t pid2 = fork();
			if(pid2 == -1){
		        	perror("fork for the second child failed\n");
			        return -1;
			}
		
			else if(pid2 == 0){	

				/*child 2 - deals with reducer.c*/
				waitpid(pid1,&status,WNOHANG|WUNTRACED);	/*child 2 is made to wait for child 1 to complete*/
				char *argsch2[] = {"./reducer",NULL};
				close(fd[1]);
				dup2(fd[0],0);	/*dup2 overwrites the standard input of child 2 process to the read end of the pipe*/
				execv(argsch2[0],argsch2);
			
			}

			else{			
				/*parent process continues here*/
			
				wait(&status);	
				close(fd[1]);	/*closing the write end of the pipe after mapper is executed */
				wait(&status);
				close(fd[0]);	/*closing the read end of the pipe after reducer is executed*/
				
			
				printf("Parent process terminated\n");
			}
	}
	}
	return 0;
}
