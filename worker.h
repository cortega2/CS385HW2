#ifndef WORKER_H_INCLUDED
#define WORKER_H_INCLUDED

struct message{
	long int type;
	int workerID;
	int sleepTime;
};
const long int HELLO = 1;
const long int BYE = 0;

#endif
