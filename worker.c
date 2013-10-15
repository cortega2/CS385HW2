#include "worker.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void wait(unsigned short semNum, int semID){						//call before using buufer
	struct sembuf buffy;
	buffy.sem_num = semNum;
	buffy.sem_op = -1;
	buffy.sem_flg = 0;
	if(semop(semID, &buffy, 1) == -1){
		perror("SEMOP failed");
		return;
	}
}
void signal(unsigned short semNum, int semID){
	struct sembuf buffy;
	buffy.sem_num = semNum;
	buffy.sem_op =  1;
	buffy.sem_flg = 0;
	if(semop(semID, &buffy, 1) == -1){
		perror("SEMOP failed");
		return;
	}
}

//parameters workerID nBuffers sleepTie msgID shmID semID
int main(int argc, char* argv[]){
	int setLock = 1;
	int semID;
	if(argc < 7)
		setLock = 0;
	else
		semID = atoi(argv[6]);

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
	int nBuffers = atoi(argv[2]);
	int workerID = atoi(argv[1]);
	unsigned int usecs = atof(argv[3]) * 1000000;
	
	if(msgsnd(msgID, &start, sizeof(struct message) - sizeof(long int), IPC_NOWAIT) < 0){
		perror("ERROR SENDING START MESSAGE!\n");
		exit(1);
	}
	
	int *sharedArray = (int *) shmat(shmID, 0, 0);
	int location = workerID;
	int read = 0;
	int i;
	int q;
	for(i = 0; i < nBuffers; i++){				//will write nBuffers times
		for(q = 0; q<workerID - 1; q++ ){			//will read workerID-1 times per write 
			//call wait
			if(setLock == 1)
				wait(location, semID);

			read = sharedArray[location];			//READ
			usleep(usecs);						//SLEEP
			int read2 = sharedArray[location];		//READ AGAIN
			//CALL signal
			if(setLock == 1)
				signal(location, semID);
			
			if(read2 != read){ 					//send message if vals are different
				struct message change;
				change.type = CHANGE;
				change.workerID = atoi(argv[1]);
				change.changedBuffer = location;
				change.initVal = read;
				change.finalVal = read2;
				
				if(msgsnd(msgID, &change, sizeof(struct message) - sizeof(long int), IPC_NOWAIT) < 0){
					printf("ERROR SENDING CHANGE MESSAGE!\n");
					exit(1);
				}	
			}			
			//update biffer location
			location = location + workerID;
			if(location >= nBuffers)
				location = location - (nBuffers);
		}
		//call wait
		if(setLock == 1)
			wait(location, semID);
		read = sharedArray[location];				//read
		usleep(usecs);							//sleep
		read = read | (1<<(workerID - 1));
		sharedArray[location] = read;				//store old val
		//call signal
		if(setLock == 1)		
			signal(location, semID);
		
		location = location + workerID;
		if(location >= nBuffers)
			location = location - (nBuffers);			
	}
	
	if(msgsnd(msgID, &end, sizeof(struct message) - sizeof(long int), IPC_NOWAIT) < 0){
		printf("ERROR SENDING END MESSAGE!\n");
		exit(1);
	}
	
	shmdt(sharedArray);
	shmctl(shmID, IPC_RMID, NULL);
	
	return 0;
}
