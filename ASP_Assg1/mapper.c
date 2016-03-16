/*
*mapper.c
*-------------------------------
* Author : Sai Vempali
* UFID   : 16141381
* email  : vishnu24@ufl.edu
*-------------------------------
*
* Mapper reads input from the file that was sent to it as an argument
* from the combiner and generates key-value pairs of the form (word,1)
* and writes them to the write end of the pipe
*/




#include <stdio.h>
#include<string.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
	
	FILE *fp;
        fp = fopen(argv[1],"r");
	if(fp == NULL){
		perror("Error in opening the input file\n");
		return -1;
	}
	
	else{
		char c[20];
		int cptr = 0;
		
		while((c[cptr] = fgetc(fp)) != EOF){		
			
			if(c[cptr] == '\n'){
				c[cptr] = '\0';
				printf("(%s,1)\n",c);
				cptr = 0;
			}
			else{
				cptr++;
			}
			
		}
		c[cptr-1] = '\0';
		fclose(fp);
	}
    return 0;
}
