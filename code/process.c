#include "headers.h"
#include "process.h"


int finishedId,detailsId;
int id;
int remainingTime;
int runTime;
int waitingTime;
int startTime,finishTime;
int memsize;


void sendDetails ()
{
    struct msgbuff message;
    message.mprocess.id=id;
    message.mprocess.remainingTime=remainingTime;
    message.mprocess.waitingTime=waitingTime;
    message.mprocess.startTime=startTime;
    message.mprocess.runTime=runTime;


    message.mtype=id;

    msgsnd(detailsId,&message,sizeof(message.mprocess),!IPC_NOWAIT);

}

void sendFinished (int terminated)
{
    struct msgbuff message;
    message.mprocess.id=id;
    message.mprocess.remainingTime=remainingTime;
    message.mprocess.waitingTime=waitingTime;
    message.mprocess.startTime=startTime;
    message.mprocess.finishTime=finishTime;
    message.mprocess.runTime=runTime;
    //message.mprocess.memsize=memsize;

    message.mtype=id;

    if (terminated == 0)
        message.mprocess.id = -1;

    msgsnd(finishedId,&message,sizeof(message.mprocess),!IPC_NOWAIT);

}


int main(int argc, char *argv[]) {
    id=atoi(argv[1]);
    remainingTime = atoi(argv[2]);
    runTime = remainingTime;
    waitingTime = 0;
    startTime = -1,finishTime = -1;
    bool isRunning = false;
    initClk();
    key_t key = ftok("keyfile", 'r');
    int runId = msgget(key, 0666 | IPC_CREAT);
    
   
   
    
    key_t keyy = ftok("keyfile", 'f'); //TO SEND BACK FINISHED PROCESSES
    finishedId= msgget(keyy, 0666 | IPC_CREAT);
    

    key_t keyyy = ftok("keyfile", 'd');//TO SEND BACK STOPPED PROCESSES TO BE ADDED BACK TO RR AND SRTN QUEUES WITH UPDATED REMAINING TIMES(AND OTHER TIMES)
    detailsId= msgget(keyyy, 0666 | IPC_CREAT);
    

    int cT = getClk();

    while(remainingTime>0)
    {
        
        if (cT != getClk())
        {
            cT = getClk();
            if (isRunning==false)
            {
                waitingTime++;
            }else{
                remainingTime--;
                if (remainingTime == 0)
                    break;
                sendFinished(0);
            }

        }

        struct msgbuff runmess;
        int receiveValue = msgrcv(runId,&runmess,sizeof(runmess.mprocess),id,IPC_NOWAIT);
        if (receiveValue!=-1)
        {
            isRunning=runmess.mprocess.isRunning;
            if (startTime==-1)
            {
                startTime=getClk();
            }
            sendDetails();
            
        }
        
    }
    finishTime=getClk();
    sendFinished(1);





    
    destroyClk(false);
    
    return 0;
}

