CS385HW2: Inter-Process Communications and Synchronization
==========================================================
Name: Carlos Ortega
netid: corteg20

Description
-----------
This is a program that was made for my OS class at UIC. The purpose of this program is to see and understand how 
memory is shared between processes and to create race conditions and use semaphores to fix them.

Files
-----
master.c
worker.h
worker.c
README.md

Compile
-------
I have included a makefile for this assignment. To compile simply use the command make.
This will compile both worker.c, worker.h, master.c

Run
---
To run the program you first need to be in the directory with both the worker.out and master.out executables
To run the program you only need to run master.out

master.out needs 6 command line arguments to run which are the following:
nBuffers, nWorkers, sleepMin, sleepMax, randSeed, lock

Command Line Arguments
----------------------
<b>nBuffer:</b> The number of shared buffers that the workers will use. It has to be a prime number < 32.
<b>nWorkers:</b> The number of workers that will write to the buffers. It has to be <b>LESS</b> than nBuffers. 
<b>sleepMin:</b> The value(float) in secs that sets the lowerlimit on the amount of time the workers should sleep.
<b>sleepMax:</b> The value(float) in secs that sets the upperlimit on the amount of time the workers should sleep.
<b>randSeed:</b> This int is used to seed the rand function. NOTE: IF 0 IS USED THEN time(NULL) WILL BE USED TO SEED.
<b>lock:</b> lock can only be 0 or 1. If <b>1</b>, then semaphores will be used. If <b>0</b>, then semaphores will NOT be used.

<b>Example:</b> ./master.out 31 28 0.01 0.011 3 0
In this example there will be 31 buffers, 28 workers. The time each worker waits will be from 0.01secs to 0.011 secs.
3 will be used to seed rand. The 0 at the end means that no semaphores will be used in this run.

Known Problems
--------------
It seems that whenever nBuffers is greater than 28, the program does not work how it was intended.
Instead of running normally, it hangs after printing "created SHR MEM, ID: SOME ID VAL ".
As of now I cannot figure out why this happens.
However this seems to be the only problem.
