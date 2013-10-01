#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	int temp;
	for(temp = 0; temp < argc; temp++){
		printf("%s\n",argv[temp]);
	}
	
	//pipes + child
	int pipe1[2];					//Parent reads/Child writes
	int pipe2[2];					//Parent writes/Child reads
	pid_t child;
	
	if(pipe(pipe1) == -1){
		printf("Pipe failed\n");
		return -1;
	}
	if(pipe(pipe2) == -1){
		printf("Pipe failed\n");
		return -1;
	}
	
	child = fork();
	//Parent
	if(child > 0){
		close(pipe1[0]);			//Not reading from pipe1
		close(pipe2[1]);			//Not writing to pipe2
		dup2(pipe2[0], 0);
		dup2(pipe1[1], 1);
		close(pipe2[0]);
		close(pipe1[1]);
	}
	//child
	else if(child == 0){
		close(pipe1[1]);			//Not writing to pipe1
		close(pipe2[0]);			//Not reading from pipe2
		dup2(pipe1[0], 0);
		dup2(pipe2[1], 1);
		close(pipe1[0]);
		close(pipe2[1]);

		char* command = {"sort", "-nr", NULL};
		execvp(command[0], command);
	}
	//something bad happened
	else{
		printf("Fork failed!\n");
		return -1;
	}
		
}
