#include "worker.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

//parameters workerID nBuffers sleepTie msgID shmID semID
int main(int argc, char* argv[]){
	struct message newM;
	newM.type = HELLO;
	newM.workerID = atoi(argv[1]);
	newM.sleepTime = atoi(argv[3]);

	int msgID = atoi(argv[4]);
	if(msgsnd(msgID, &newM, sizeof(struct message) - sizeof(long int), IPC_NOWAIT) < 0){
		printf("ERROR SENDING MESSAGE!\n");
		exit(1);
	}
	
}
