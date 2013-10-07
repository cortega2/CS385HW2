#include "worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>

//for message queue
#define MSG_RW 0600

int rmv(int id){
	char theID[80];
	sprintf(theID, "%d", id);
	pid_t child;
	child = fork();
	//something bad happened
	if(child < 0){
		printf("ERROR FORKING\n");
		return -1;
	}
	//child
	else if(child == 0){
		execlp("ipcrm","ipcrm", "-q",theID, NULL);
		printf("IPCRM failed\n");
		return -1;
	}
	wait(NULL);
	return 0;
}

int main(int argc, char* argv[]){
	if(argc<7){
		printf("Not enought arguments\n");
		return -1;
	}
	int randSeed = atoi(argv[5]);
	int sleepMax = atoi(argv[4]);
	int sleepMin = atoi(argv[3]);
	int nWorkers = atoi(argv[2]);
	char readBuff[256];
	
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
	
	//seperate sleep time using strtok and convert them to ints to be stored
	int i = 0;
	char* tmp = NULL;
	int sleepTimes[nWorkers];
	tmp = strtok(readBuff, "\n");
	while(tmp != NULL){
		sleepTimes[i] = atoi(tmp);
		tmp = strtok(NULL, "\n");
		i++;
	}
	
	//Part 2: Create message queue and fork of workers to write messages to the message queue
	int msgID = msgget(IPC_PRIVATE, 0600);
	if(msgID == -1){
		printf("msgget error\n");
		return -1;
	}
	else
		printf("Created a MSG queue, ID: %d\n", msgID);

	pid_t workersPIDs[nWorkers];
	char mID[80];
	sprintf(mID, "%d", msgID);
	char shmID[80] = "000";							//CHANGE LATER TO SOMETHING ELSE
	char semID[80] = "000";							//CHANGE LATER TO SOMETHING ELSE
	for(i = 0; i < nWorkers; i++){
		char workerID[80];
		char sleepT[80];
		workersPIDs[i] = fork();
		sprintf(workerID, "%d", i);
		sprintf(sleepT, "%d", sleepTimes[i]);	
		
		//Parent
		if(workersPIDs[i] >0 ){
		}
		//Child
		else if(workersPIDs[i] == 0){
			execlp("./worker.out", "./worker.out", workerID, argv[1], sleepT, mID, shmID, semID, NULL);
		}
		//something bad happened
		else{
			printf("Something bad happened when forking!\n");
			return -1;
		}
	}

	//Read messages from the children workers
	for(i = 0; i< nWorkers; i++){
		struct message rMessage;
		if(msgrcv(msgID, &rMessage, sizeof(struct message) - sizeof(long int), HELLO, 0) < 0){
			printf("ERROR RECIEVING MESSAGE\n");
			return -1;
		}
		printf("Message from worker[%d]\nSleep Time:%d\n", rMessage.workerID, rMessage.sleepTime);
	}
	rmv(msgID);
	//wait for the children workers
	for(i = 0; i< nWorkers; i++)
		wait(NULL);
	
	printf("DONE WAITING\n");
			
}
