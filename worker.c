#include "worker.h"

//parameters workerID nBuffers sleepTie msgID shmID semID
int main(int argc, char* argv[]){
	struct message newM;
	newM.type = HELLO;

	printf("%d\n", newM.type);
}
