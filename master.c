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
#define MSG_RW 0600 | IPC_CREAT

int rmv(int idm, int toRMV){
	char *qs = NULL;
	if(toRMV == 0) qs =	"-q";		//remove queue
	else qs = "-m";				//remove shared mem

	char theID[80];
	sprintf(theID, "%d", idm);
	pid_t child;
	child = fork();
	//something bad happened
	if(child < 0){
		printf("ERROR FORKING\n");
		return -1;
	}
	//child
	else if(child == 0){
		execlp("ipcrm","ipcrm", qs,theID, NULL);
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
	float sleepMax = atof(argv[4]);
	float sleepMin = atof(argv[3]);
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
	int shmArray[nWorkers];							//Figure out if there is a better way to do this
	int shrMemID = shmget(IPC_PRIVATE, sizeof(shmArray), MSG_RW);
	if(shrMemID == -1){
		printf("shmget error\n");
		return -1;
	}
	else
		printf("Created SHR_MEM, ID: %d\n", shrMemID);

	//Initialize shared array
	int *sharedArray = (int *) shmat(shrMemID, 0, 0);
	for(i = 0; i<nWorkers; i++)
		sharedArray[i] = 0;
	printf("%d\n", sharedArray[0]);
	
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
		sprintf(sleepT, "%f", sleepTimes[i]);	
		
		//printf("%s\n", sleepT);
			
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
		printf("Message from worker[%d]\nSleep Time:%f\n", rMessage.workerID, rMessage.sleepTime);
	}
	rmv(msgID, 0);
	shmdt(sharedArray);
	shmctl(shrMemID, IPC_RMID, NULL);

	//rmv(shrMemID, 1);					//MIGHT NOT BE NEEDED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//wait for the children workers
	for(i = 0; i< nWorkers; i++)
		wait(NULL);
	
	printf("DONE WAITING\n");
			


}
