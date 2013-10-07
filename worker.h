#ifndef WORKER_H_INCLUDED
#define WORKER_H_INCLUDED

struct message{
	int type;
	int workerID;
	int sleepTime;
};
const int HELLO = 1;
const int BYE = 0;

#endif
