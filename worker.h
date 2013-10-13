#ifndef WORKER_H_INCLUDED
#define WORKER_H_INCLUDED

struct message{
	long int type;
	int workerID;
	float sleepTime;
	int changedBuffer;
	int initVal;
	int finalVal;
};
const long int CHANGE = 3; 
const long int HELLO = 2;
const long int BYE = 1;

#endif
