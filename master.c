#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	if(argc<7){
		printf("Not enought arguments\n");
		return -1;
	}
	int randSeed = atoi(argv[5]);
	int sleepMax = atoi(argv[4]);
	int sleepMin = atoi(argv[3]);
	int nWorkers = atoi(argv[2]);
	
	if(randSeed == 0)
		srand(time(NULL));
	else
		srand(randSeed);	

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
		int savedSTDOUT = dup(1);
		char buff[256];
		char readBuff[256];

		//char* testB = "1\n10\n3\n";

		int t;
		int b=0;
		for(t =0; t < nWorkers; t++){
			int b2 = sprintf(buff + b, "%d\n", (rand() % sleepMax + sleepMin));
			b = b + b2;
		}
		printf("%s", buff);
			
		close(pipe1[0]);					//Not reading from pipe1
		close(pipe2[1]);					//Not writing to pipe2
		
		write(pipe1[1], buff, strlen(buff));
		close(pipe1[1]);
		
		while(read(pipe2[0], readBuff, sizeof(readBuff)) != 0){
		}
		close(pipe2[0]);
		
		wait(NULL);
		printf("\nReading:\n%s", readBuff);
	}
	//child
	else if(child == 0){
		close(pipe1[1]);					//Not writing to pipe1
		close(pipe2[0]);					//Not reading from pipe2
		
		dup2(pipe1[0], 0);
		close(pipe1[0]);
		dup2(pipe2[1], 1);
		dup2(pipe2[1], 2);	
		close(pipe2[1]);
		
		execlp("sort", "sort", "-nr",NULL);
		printf("Something bad happened\n");
		return;
	}
	//something bad happened
	else{
		printf("Fork failed!\n");
		return -1;
	}		
}
