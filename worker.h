#ifndef WORKER_H_INCLUDED
#define WORKER_H_INCLUDED

struct message{
	long int type;
	int workerID;
	float sleepTime;
};
const long int HELLO = 2;
const long int BYE = 1;

#endif
