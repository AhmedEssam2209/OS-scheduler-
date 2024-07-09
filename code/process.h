
#ifndef PROCESS_H  
#define PROCESS_H
typedef struct
{
    int id;
    int arrival;
    int runTime;
    int priority;
    int waitingTime;
    int remainingTime;
    int startTime;
    int finishTime;
    int memsize;
    bool isRunning;
    bool startedbefore;
} Process;

void constProcess(Process *process, int id, int arrival, int runTime, int priority,int msize)
{
    
    process->id = id;
    process->arrival = arrival;
    process->runTime = runTime;
    process->priority = priority;
    process->remainingTime = runTime;
    process->waitingTime = 0;
    process->startTime = -1;
    process->finishTime = -1;
    process->isRunning = false;
    process->memsize = msize;
}


struct msgbuff {
    long mtype;
    Process mprocess;
};

#endif  