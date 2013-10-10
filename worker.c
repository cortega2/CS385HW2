#include "worker.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//parameters workerID nBuffers sleepTie msgID shmID semID
int main(int argc, char* argv[]){
	struct message end;
	end.type = BYE;
	end.workerID = atoi(argv[1]);
	end.sleepTime = atof(argv[3]);

	struct message start;
	start.type = HELLO;
	start.workerID = atoi(argv[1]);
	start.sleepTime = atof(argv[3]);	

	int shmID = atoi(argv[5]);
	int msgID = atoi(argv[4]);
	
	if(msgsnd(msgID, &start, sizeof(struct message) - sizeof(long int), IPC_NOWAIT) < 0){
		printf("ERROR SENDING START MESSAGE!\n");
		exit(1);
	}	
	int *sharedArray = (int *) shmat(shmID, 0, 0);
	sharedArray[end.workerID] = end.workerID;
	if(msgsnd(msgID, &end, sizeof(struct message) - sizeof(long int), IPC_NOWAIT) < 0){
		printf("ERROR SENDING END MESSAGE!\n");
		exit(1);
	}
	
	shmdt(sharedArray);
	shmctl(shmID, IPC_RMID, NULL);

	return 0;
}
