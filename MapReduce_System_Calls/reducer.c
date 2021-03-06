/**
*reducer.c
*-------------------------------
* Author : Sai Vempali
* UFID   : 16141381
* email  : vishnu24@ufl.edu
*-------------------------------
*
* Reducer reads input from the read end of the pipe and it 
* uses 2D array args to store the words and a 1D array count
* to maintain the count of each word.
*
* Word stored at location pointed by args[i] has its corresponding
* word count value stored at location pointed by count[i].
*
* Each time a word starts with a new letter different from the starting 
* letter of the previous, it means that we will no longer encounter words 
* starting from the previous word's starting letter as the words are in sorted 
* order.
*
* So, when a word starting with a new letter is encountered, all the words in the args
* are printed in the (word,total_count) format and the args and count array memory id freed
* and fresh memory is created.
*
* Thus, Reducer outputs the key-value pair immediately when it sees a word that starts with a
* different letter. 
*
* To use memory efficiently, the reducer frees the memory allocated to a set of words starting 
* with the same letter once they have been printed to the standard output.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main(int argc, char *argv[]){
	
	char **args;
	int *count;
	int argsRow=0;
	char *buf;
	size_t bufsize = 20,argsSize=10;
	int length;
	buf = (char *)malloc(bufsize+1);
	args = malloc(argsSize*sizeof(char*));
	count = malloc(argsSize*sizeof(int*));	

	int i;
	for(i=0; i<argsSize; i++)
		count[i] = 0;

	while(1){
		length = getline(&buf, &bufsize, stdin);	
		int currWordLen=0,bufRow=1;
		while(buf[bufRow] != ',')
			currWordLen++,bufRow++;
		if(length == -1){
			int runner = 0;
			while(runner < argsRow){
                                printf("(%s,%d)\n",args[runner],count[runner]);
	                        runner++;
                        }

//			perror("No words available | Pipe is empty\n");
			exit(0);		
		}
		
		else{
			if(argsRow == 10){

				argsSize = argsSize+10;
				args = realloc(args,argsSize*sizeof(char*));				
				count = realloc(count,argsSize*sizeof(int*));
				int i;
				for(i=argsSize-10; i < argsSize; i++)
					count[i] = 0;
			}
			if(argsRow == 0){

				args[argsRow] = malloc((currWordLen+1)*sizeof(char*));
				memcpy(args[argsRow],buf+1,currWordLen);
				args[argsRow][currWordLen] = '\0';
				count[argsRow]++;	
				argsRow++;			
			}
			else{

				int runner = 0;
				char *temp = malloc((currWordLen+1)*sizeof(char*));
				memcpy(temp,buf+1,currWordLen);
				temp[currWordLen] = '\0';

				if(temp[0] != args[argsRow-1][0] && temp[0] != '\n'){

					while(runner < argsRow){
						printf("(%s,%d)\n",args[runner],count[runner]);
						runner++;
					}
					runner = 0;
					free(args);
					free(count);
					argsSize = 10;
					argsRow = 0;
					args = malloc(argsSize*sizeof(char*));
        				count = malloc(argsSize*sizeof(int*));
        				int i;
        				for(i=0; i<argsSize; i++)
			                	count[i] = 0;
					args[argsRow] = malloc((currWordLen+1)*sizeof(char*));
                               		strcpy(args[argsRow],temp);
                                	args[argsRow][currWordLen] = '\0';
                                	count[argsRow]++;
	                                argsRow++;
					continue;

				}
				runner = 0;
				while(runner < argsRow)
        			{
        			
        				if(strcmp(temp,args[runner]) == 0)
        				{	
        					break;
        				}
        				runner++;
        			}

				if(runner <argsRow){
					count[runner]++;
				}
				else
        			{
					args[argsRow] = malloc((currWordLen+1)*sizeof(char*));
        				strcpy(args[argsRow],temp);
					args[argsRow][currWordLen] = '\0';
        				count[argsRow]++;
        				argsRow++;
	        		}
			}
		}
	}	
	free(args);
	free(count);

	return 0;
}
