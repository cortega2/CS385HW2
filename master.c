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
#include <sys/sem.h>

//for message queue
#define MSG_RW 0600 | IPC_CREAT
union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

int main(int argc, char* argv[]){
	printf("Carlos Ortega	corteg20\n");	

	if(argc<7){
		printf("Not enought arguments\n");
		return -1;
	}
	int randSeed = atoi(argv[5]);
	float sleepMax = atof(argv[4]);
	float sleepMin = atof(argv[3]);
	int nWorkers = atoi(argv[2]);
	int nBuffers = atoi(argv[1]);
	int numSems = atoi(argv[6]);
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

		int t;
		int b=0;
		for(t =0; t < nWorkers; t++){
			float r = (float)rand() / (float)RAND_MAX;
    			float diff = sleepMax - sleepMin;
	    		r = r * diff;
			r = sleepMin + r;
			int b2 = sprintf(buff + b, "%f\n", r);
			b = b + b2;
		}
		close(pipe1[0]);					//Not reading from pipe1
		close(pipe2[1]);					//Not writing to pipe2
		
		write(pipe1[1], buff, strlen(buff));
		close(pipe1[1]);
		
		while(read(pipe2[0], readBuff, sizeof(readBuff)) != 0){
		}
		close(pipe2[0]);
		wait(NULL);
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
	
	//seperate sleep time using strtok and convert them to floats to be stored
	int i = 0;
	char* tmp = NULL;
	float sleepTimes[nWorkers];
	tmp = strtok(readBuff, "\n");
	while(tmp != NULL){
		sleepTimes[i] = atof(tmp);
		tmp = strtok(NULL, "\n");
		i++;
	}


	
	//Part 2: Create message queue and fork of workers to write messages to the message queue
	int msgID = msgget(IPC_PRIVATE, MSG_RW);
	if(msgID == -1){
		printf("msgget error\n");
		return -1;
	}
	else
		printf("Created a MSG queue, ID: %d\n", msgID);
	
	//Part 3: Create and test shared memory
	int shmArray[nBuffers];							//Figure out if there is a better way to do this
	int shrMemID = shmget(IPC_PRIVATE, sizeof(shmArray), MSG_RW);
	if(shrMemID == -1){
		printf("shmget error\n");
		return -1;
	}
	else{
		printf("Created SHR_MEM, ID: %d\n", shrMemID);
	}

	//Initialize shared array
	int *sharedArray = (int *) shmat(shrMemID, 0, 0);
	for(i = 0; i<nBuffers; i++)
		sharedArray[i] = 0;

	

	//Part 5: create Semaphores
	int intSemID = 0;
	intSemID = semget(IPC_PRIVATE, nBuffers, MSG_RW);
	union semun semUnion;
	semUnion.val = 1;
	
	for(i =0; i < nBuffers; i++)
		semctl(intSemID, i, SETVAL, semUnion);
		
	//Fork of the workers	
	pid_t workersPIDs[nWorkers];
	char mID[80];
	sprintf(mID, "%d", msgID);
	char shmID[80];
	sprintf(shmID, "%d", shrMemID);
	
	char semID[80];
	if(numSems > 0)
		sprintf(semID, "%d", intSemID);
	else
		sprintf(semID, "%d", 0);
	
	for(i = 0; i < nWorkers; i++){
		char workerID[80];
		char sleepT[80];
		workersPIDs[i] = fork();
		sprintf(workerID, "%d", i+1);
		sprintf(sleepT, "%f", sleepTimes[i]);	
			
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
	//Read messages from the children workers and wait if received end message
	i = nWorkers;
	while(i){
		struct message rMessage;
		if(msgrcv(msgID, &rMessage, sizeof(struct message) - sizeof(long int), 0, 0) < 0){
			printf("ERROR RECIEVING START MESSAGE\n");
			return -1;
		}
		if(rMessage.type == HELLO)
			printf("Start message from worker[%d]	Sleep Time:%f\n", rMessage.workerID, rMessage.sleepTime);
		if(rMessage.type == CHANGE){
			int change = rMessage.initVal ^ rMessage.finalVal;
			printf("Worker[%d] reports change in Buffer[%d]\nInitial:%d   Final:%d	Bad bits:", 
				rMessage.workerID, rMessage.changedBuffer, rMessage.initVal, rMessage.finalVal);
			
			int q;	
			for(q = 0; q<nWorkers; q++){
				if(change & (1<<q))
					printf("%d	", q);
			}
			printf("\n\n");
		}
		else if(rMessage.type == BYE){
			printf("End message from worker[%d]	Sleep Time:%f\n", rMessage.workerID, rMessage.sleepTime);
			i--;
			wait(NULL);
		}
	}
	
	//Print out the contents of the buffer
	int expVal = (1 << nWorkers)-1;

	printf("\nExpected Val: %d\n", expVal);
	int q;
	for(i = 0; i< nBuffers; i++){
		if(sharedArray[i] != expVal){
			int change = sharedArray[i] ^ expVal;
			printf("Write Error in Buffer[%d]=%d     Bad bits:", i, sharedArray[i]);
			for(q = 0; q<nWorkers; q++){
				if(change & (1<<q))
					printf("%d    ", q);
			}
			printf("\n");
		}
	}
	
	shmdt(sharedArray);
	shmctl(shrMemID, IPC_RMID, NULL);
	msgctl(msgID, IPC_RMID, NULL);
	semctl(intSemID, nBuffers, IPC_RMID);
	
	
	printf("DONE WAITING\n");
	

}
